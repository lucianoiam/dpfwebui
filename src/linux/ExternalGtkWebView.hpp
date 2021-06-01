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

#include <cstdint>
#include <sys/types.h>

#include "extra/Thread.hpp"

#include "WebViewInterface.hpp"
#include "ipc.h"
#include "helper.h"

START_NAMESPACE_DISTRHO

class ExternalGtkWebView : public WebViewInterface
{
friend class IpcReadThread;

public:
    ExternalGtkWebView();
    ~ExternalGtkWebView();

    void navigate(String url) override;
    void reparent(uintptr_t windowId) override;
    void resize(const Size<uint>& size) override;

private:
    ipc_t* ipc() const { return fIpc; }
    int    ipcWrite(helper_opcode_t opcode, const void *payload, int payloadSize); 
    void   ipcReadCallback(const tlv_t& message);

    int     fPipeFd[2][2];
    pid_t   fPid;
    ipc_t*  fIpc;
    Thread* fIpcThread;

};

class IpcReadThread : public Thread
{
public:
    IpcReadThread(ExternalGtkWebView& view);
    
    void run() override;

private:
    ExternalGtkWebView& fView;

};

END_NAMESPACE_DISTRHO

#endif  // EXTERNALGTKWEBVIEW_HPP
