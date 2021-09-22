/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "IpcInterface.hpp"

#include "macro.h"

USE_NAMESPACE_DISTRHO

IpcInterface::IpcInterface(int fdr, int fdw, long int readTimeoutUsec)
    : fTimeout(readTimeoutUsec)
{
    ipc_conf_t conf;
    conf.fd_r = fdr;
    conf.fd_w = fdw;
    fIpc = ipc_init(&conf); 
}

IpcInterface::~IpcInterface()
{
    if (fIpc != 0) {
        ipc_destroy(fIpc);
    }
}

int IpcInterface::read(tlv_t* packet) const
{
    struct timeval tv;  
    fd_set rfds;
    int fd = ipc_get_config(fIpc)->fd_r;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = fTimeout;

    int rc = select(fd + 1, &rfds, 0, 0, &tv);

    if (rc == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Failed select() on IPC channel");
        return -1;
    }

    if (rc == 0) {
        return -1; // no fd ready
    }

    if (ipc_read(fIpc, packet) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not read from IPC channel");
        return -1;
    }

    return 0;
}

int IpcInterface::write(msg_opcode_t opcode) const
{
    return write(opcode, 0, 0);
}

int IpcInterface::write(msg_opcode_t opcode, String& str) const
{
    const char *cStr = static_cast<const char *>(str);
    return write(opcode, cStr, strlen(cStr) + 1);
}

int IpcInterface::write(msg_opcode_t opcode, const void* payload, int payloadSize) const
{
    tlv_t packet;

    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    int rc = ipc_write(fIpc, &packet);

    if (rc == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not write to IPC channel");
    }

    return rc;
}
