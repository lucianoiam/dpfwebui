/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#ifndef EXTERNALGTKWEBWIDGET_HPP
#define EXTERNALGTKWEBWIDGET_HPP

#include <cstdint>
#include <sys/types.h>

#include "extra/Thread.hpp"

#include "AbstractWebWidget.hpp"

#include "extra/ipc.h"
#include "helper.h"

START_NAMESPACE_DISTRHO

class ExternalGtkWebWidget : public AbstractWebWidget
{
friend class IpcReadThread;

public:
    ExternalGtkWebWidget(Widget *parentWidget);
    ~ExternalGtkWebWidget();

    void setBackgroundColor(uint32_t rgba) override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

    void setKeyboardFocus(bool focus) override;

protected:
    void onResize(const ResizeEvent& ev) override;
    void onPositionChanged(const PositionChangedEvent& ev) override;
    bool onKeyboard(const KeyboardEvent& ev) override;

private:
    ipc_t* ipc() const { return fIpc; }
    int    ipcWriteString(helper_opcode_t opcode, String str) const;
    int    ipcWrite(helper_opcode_t opcode, const void *payload, int payloadSize) const; 
    void   ipcReadCallback(const tlv_t& message);

    void   handleHelperScriptMessage(const char *payload, int payloadSize);

    int     fPipeFd[2][2];
    pid_t   fPid;
    ipc_t*  fIpc;
    Thread* fIpcThread;

};

class IpcReadThread : public Thread
{
public:
    IpcReadThread(ExternalGtkWebWidget& view);
    
    void run() override;

private:
    ExternalGtkWebWidget& fView;

};

END_NAMESPACE_DISTRHO

#endif  // EXTERNALGTKWEBWIDGET_HPP
