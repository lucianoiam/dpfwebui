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

#ifndef CHILD_PROCESS_WEBVIEW_HPP
#define CHILD_PROCESS_WEBVIEW_HPP

#include <cstdint>
#include <functional>

#include <sys/types.h>
#include <X11/Xlib.h>

#include "extra/Thread.hpp"

#include "AbstractWebView.hpp"
#include "IpcChannel.hpp"

START_NAMESPACE_DISTRHO

class ChildProcessWebView : public AbstractWebView
{
public:
    ChildProcessWebView();
    virtual ~ChildProcessWebView();

    float getDisplayScaleFactor() { return fDisplayScaleFactor; }

    void realize() override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

protected:
    void onSize(uint width, uint height) override;
    void onKeyboardFocus(bool focus) override;

private:
    void ipcReadCallback(const tlv_t& message);
    void handleInit(float displayScaleFactor);
    void handleHelperScriptMessage(const char *payload, int payloadSize);

    ::Display*  fDisplay;
    ::Window    fBackground;
    int         fPipeFd[2][2];
    pid_t       fPid;
    IpcChannel* fIpc;
    Thread*     fIpcThread;
    bool        fChildInit;
    float       fDisplayScaleFactor;

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
