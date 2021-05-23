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

#ifndef EXTERNALGTKWEBVIEW_HPP
#define EXTERNALGTKWEBVIEW_HPP

#include "../WebViewInterface.hpp"

#include <cstdint>
#include <sys/types.h>

#include "extra/Thread.hpp"

#include "ipc.h"

START_NAMESPACE_DISTRHO

class ExternalGtkWebView : public WebViewInterface
{
friend class IpcReadThread;

public:
    ExternalGtkWebView();
    ~ExternalGtkWebView();

    virtual void reparent(uintptr_t parentWindowId) override;

private:
    bool isRunning() { return fPid != -1; }
    int  spawn();
    void terminate();

    ipc_t* ipc() const { return fIpc; }
    int    send(char opcode, const void *data, int size); 
    void   readCallback(const ipc_msg_t& message) const;

    int     fPipeFd[2][2];
    pid_t   fPid;
    ipc_t*  fIpc;
    Thread* fIpcThread;

};

typedef ExternalGtkWebView PlatformWebView;

class IpcReadThread : public Thread
{
public:
    IpcReadThread(const ExternalGtkWebView& view) : Thread("ipc_read"), fView(view) {}

    void run() override;

private:
    const ExternalGtkWebView& fView;

};

END_NAMESPACE_DISTRHO

#endif  // EXTERNALGTKWEBVIEW_HPP
