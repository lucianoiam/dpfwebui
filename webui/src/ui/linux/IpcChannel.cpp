/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "IpcChannel.hpp"

#include <errno.h>
#include <sys/select.h>

USE_NAMESPACE_DISTRHO

IpcChannel::IpcChannel(int fdr, int fdw, int readTimeoutMs, int writeTimeoutMs)
{
    ipc_conf_t conf;
    conf.fd_r = fdr;
    conf.fd_w = fdw;
    fIpc = ipc_init(&conf);

    fReadTimeoutMs = readTimeoutMs;
    fWriteTimeoutMs = writeTimeoutMs;
}

IpcChannel::~IpcChannel()
{
    if (fIpc != nullptr) {
        ipc_destroy(fIpc);
    }
}

int IpcChannel::getFdRead() const
{
    return ipc_get_config(fIpc)->fd_r;
}

int IpcChannel::getFdWrite() const
{
    return ipc_get_config(fIpc)->fd_w;
}

int IpcChannel::read(tlv_t* packet) const
{
    if ((fReadTimeoutMs >= 0) && (wait(getFdRead(), fReadTimeoutMs) == -1)) {
        return -1;
    }

    if (ipc_read(fIpc, packet) == -1) {
        d_stderr("IpcChannel : read error - %s", strerror(errno));
        return -1;
    }

    return 0;
}

int IpcChannel::write(msg_opcode_t opcode) const
{
    return write(opcode, 0, 0);
}

int IpcChannel::write(msg_opcode_t opcode, String& str) const
{
    const char *cStr = static_cast<const char *>(str);
    return write(opcode, cStr, strlen(cStr) + 1);
}

int IpcChannel::write(msg_opcode_t opcode, const void* payload, int payloadSize) const
{
    if ((fWriteTimeoutMs >= 0) && wait(getFdWrite(), fWriteTimeoutMs) == -1) {
        d_stderr("IpcChannel : write timeout");
        return -1;
    }

    tlv_t packet;

    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    if (ipc_write(fIpc, &packet) == -1) {
        d_stderr("IpcChannel : write error - %s", strerror(errno));
        return -1;
    }

    return 0;
}

int IpcChannel::wait(int fd, int timeoutMs)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000L * (long)timeoutMs;

    const int rc = select(fd + 1, &fds, 0, 0, &tv);

    if (rc == -1) {
        d_stderr("IpcChannel : select error - %s", strerror(errno));
        return -1;
    }

    if (rc == 0) {
        return -1; // timeout
    }

    return 0;
}
