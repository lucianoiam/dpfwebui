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

#ifndef EXTERNALGTKWEBVIEWUI_HPP
#define EXTERNALGTKWEBVIEWUI_HPP

#include <cstdint>
#include <sys/types.h>

#include "extra/Thread.hpp"

#include "../WebUI.hpp"
#include "ipc.h"

START_NAMESPACE_DISTRHO

class ExternalGtkWebViewUI : public WebUI
{
friend class IpcReadThread;

public:
    ExternalGtkWebViewUI();
    ~ExternalGtkWebViewUI();

    void reparent(uintptr_t windowId) override;

    void parameterChanged(uint32_t index, float value) override;

private:
    bool isRunning() { return fPid != -1; }
    int  spawn();
    void terminate();

    ipc_t* ipc() const { return fIpc; }
    int    ipcWrite(char opcode, const void *data, int size); 
    void   ipcReadCallback(const ipc_msg_t& message);

    int     fPipeFd[2][2];
    pid_t   fPid;
    ipc_t*  fIpc;
    Thread* fIpcThread;

};

class IpcReadThread : public Thread
{
public:
    IpcReadThread(ExternalGtkWebViewUI& view);
    
    void run() override;

private:
    ExternalGtkWebViewUI& fView;

};

END_NAMESPACE_DISTRHO

#endif  // EXTERNALGTKWEBVIEWUI_HPP
