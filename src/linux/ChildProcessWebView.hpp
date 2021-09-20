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
#include <sys/types.h>
#include <X11/Xlib.h>

#include "extra/Thread.hpp"

#include "AbstractWebView.hpp"

#include "ipc.h"
#include "ipc_message.h"

START_NAMESPACE_DISTRHO

class ChildProcessWebView : public AbstractWebView
{
friend class IpcReadThread;

public:
    ChildProcessWebView();
    virtual ~ChildProcessWebView();

    void realize() override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

protected:
    void onSize(uint width, uint height) override;
    void onKeyboardFocus(bool focus) override;

private:
    ipc_t* ipc() const { return fIpc; }
    int    ipcWrite(msg_opcode_t opcode, const void *payload, int payloadSize) const; 
    int    ipcWriteOpcode(msg_opcode_t opcode) const;
    int    ipcWriteString(msg_opcode_t opcode, String str) const;
    void   ipcReadCallback(const tlv_t& message);

    void   handleHelperScriptMessage(const char *payload, int payloadSize);

    ::Display* fDisplay;
    ::Window   fBackground;
    int        fPipeFd[2][2];
    pid_t      fPid;
    ipc_t*     fIpc;
    Thread*    fIpcThread;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChildProcessWebView)

};

class IpcReadThread : public Thread
{
public:
    IpcReadThread(ChildProcessWebView& view);
    
    void run() override;

private:
    ChildProcessWebView& fView;

};

END_NAMESPACE_DISTRHO

#endif  // CHILD_PROCESS_WEBVIEW_HPP
