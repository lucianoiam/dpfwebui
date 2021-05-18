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

#include "HelperIpc.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream> // FIXME - dev only
#include "extra/Sleep.hpp" // FIXME - dev only

USE_NAMESPACE_DISTRHO

HelperIpc::HelperIpc()
    : Thread("helper_ipc")
{
    mkfifo("/tmp/helper1", 0666);
    fReadPipe = open("/tmp/helper1", O_RDWR);
    mkfifo("/tmp/helper2", 0666);
    fWritePipe = open("/tmp/helper2", O_RDWR);
}

HelperIpc::~HelperIpc()
{
    close(fReadPipe);
    close(fWritePipe);
}

void HelperIpc::run()
{
    while (1) {
        fd_set rfds;
        struct timeval tv;
        FD_ZERO(&rfds);
        FD_SET(fReadPipe, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        int retval = select(fReadPipe + 1, &rfds, NULL, NULL, &tv);

        if (shouldThreadExit()) {
            break;
        }

        if (retval == 0) {
            continue;   // select() timeout
        }

        int opcode, size;
        read(fReadPipe, &opcode, sizeof(opcode));
        read(fReadPipe, &size, sizeof(size));
        void *payload = malloc(size);
        read(fReadPipe, payload, size);
        std::cout << "Plugin Rx: " << String((char*)payload) << std::endl;
        free(payload);
    }
}

void HelperIpc::sendString(int opcode, const String &s)
{
    const char* cs = static_cast<const char*>(s);
    send(opcode, cs, 1 + strlen(cs));

}
void HelperIpc::send(int opcode, const void *payload, int size)
{
    // TO DO: move this to ipc.c
    write(fWritePipe, &opcode, sizeof(opcode));
    write(fWritePipe, &size, sizeof(size));
    write(fWritePipe, payload, size);
}
