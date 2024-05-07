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

#ifndef IPC_CHANNEL_HPP
#define IPC_CHANNEL_HPP

#include "distrho/extra/LeakDetector.hpp"
#include "distrho/extra/String.hpp"

#include "ipc.h"
#include "ipc_message.h"

START_NAMESPACE_DISTRHO

class IpcChannel
{
public:
    // -1 means block indefinitely
    IpcChannel(int fdr, int fdw, int readTimeoutMs = -1, int writeTimeoutMs = -1);
    virtual ~IpcChannel();

    int getFdRead() const;
    int getFdWrite() const;

    int read(tlv_t* packet) const;

    int write(msg_opcode_t opcode) const;
    int write(msg_opcode_t opcode, String& str) const;
    int write(msg_opcode_t opcode, const void* payload, int payloadSize) const; 

private:
    static int wait(int fd, int timeoutMs);

    ipc_t* fIpc;
    int    fReadTimeoutMs;
    int    fWriteTimeoutMs;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IpcChannel)

};

END_NAMESPACE_DISTRHO

#endif  // IPC_CHANNEL_HPP
