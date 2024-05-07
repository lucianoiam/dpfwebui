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

#ifndef CHILD_PROCESS_WEBVIEW_HPP
#define CHILD_PROCESS_WEBVIEW_HPP

#include <cstdint>
#include <functional>

#include <sys/types.h>
#include <X11/Xlib.h>

#include "distrho/extra/Thread.hpp"

#include "../WebViewBase.hpp"
#include "IpcChannel.hpp"

START_NAMESPACE_DISTRHO

class ChildProcessWebView : public WebViewBase
{
public:
    ChildProcessWebView(String userAgentComponent = String());
    virtual ~ChildProcessWebView();

    float getDevicePixelRatio() override;
    
    void realize() override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

protected:
    void onSize(uint width, uint height) override;
    void onKeyboardFocus(bool focus) override;

private:
    void ipcReadCallback(const tlv_t& message);
    void handleInit(float devicePixelRatio);
    void handleHelperScriptMessage(const char *payloadBytes, int payloadSize);
    void cleanup();

    String      fUserAgent;
    ::Display*  fDisplay;
    ::Window    fBackground;
    int         fPipeFd[2][2];
    pid_t       fPid;
    IpcChannel* fIpc;
    Thread*     fIpcThread;
    float       fDevicePixelRatio;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChildProcessWebView)

};

class IpcReadThread : public Thread
{
public:
    typedef std::function<void(const tlv_t& message)> IpcReadCallback;

    IpcReadThread(IpcChannel* ipc, IpcReadCallback callback);
    
    void run() override;

private:
    IpcChannel*     fIpc;
    IpcReadCallback fCallback;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IpcReadThread)

};

END_NAMESPACE_DISTRHO

#endif  // CHILD_PROCESS_WEBVIEW_HPP
