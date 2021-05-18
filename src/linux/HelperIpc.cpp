/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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
