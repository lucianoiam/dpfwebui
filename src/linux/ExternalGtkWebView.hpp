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

#ifndef EXTERNALGTKWEBVIEW_HPP
#define EXTERNALGTKWEBVIEW_HPP

#include "../WebViewInterface.hpp"

#include <cstdint>
#include <sys/types.h>

#include "extra/String.hpp"
#include "extra/Thread.hpp"

#include "ipc.h"

START_NAMESPACE_DISTRHO

class HelperIpcReadThread : public Thread
{
public:
    HelperIpcReadThread() : Thread("ipc_read") {}

    void setIpc(ipc_t *ipc) { fIpc = ipc; };

    virtual void run() override;

    // TODO: callback function

private:
    ipc_t* fIpc;

};

class ExternalGtkWebView : public WebViewInterface
{
public:
    ExternalGtkWebView() : fPipeFd(), fPid(0), fIpc(0) {}
    ~ExternalGtkWebView();
    
    virtual void reparent(uintptr_t parentWindowId) override;

private:
    bool isRunning() { return fPid != 0; }
    int  spawn();
    void terminate();

    int send(char opcode, const void *data, int size);

    int                 fPipeFd[2][2];
    pid_t               fPid;
    ipc_t*              fIpc;
    HelperIpcReadThread fIpcThread;

};

typedef ExternalGtkWebView PlatformWebView;

END_NAMESPACE_DISTRHO

#endif  // EXTERNALGTKWEBVIEW_HPP
