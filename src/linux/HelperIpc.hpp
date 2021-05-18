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

#ifndef HELPERIPC_HPP
#define HELPERIPC_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include "extra/Thread.hpp"

START_NAMESPACE_DISTRHO

class HelperIpc : public Thread
{
public:
    HelperIpc();
    virtual ~HelperIpc() override;

    // TO DO: receive callback

    void sendString(int opcode, const String &s);

protected:
    virtual void run() override;

private:
    void send(int opcode, const void *payload, int size);

    int fReadPipe;
    int fWritePipe;

};

END_NAMESPACE_DISTRHO

#endif  // HELPERIPC_HPP
