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

#include "ExternalGtkWebViewUI.hpp"

#include <cstdio>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "Platform.hpp"
#include "log.h"
#include "macro.h"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

extern char **environ;

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new ExternalGtkWebViewUI;
}

ExternalGtkWebViewUI::ExternalGtkWebViewUI()
    : fPid(-1)
    , fIpc(nullptr)
    , fIpcThread(nullptr)
{
    fPipeFd[0][0] = fPipeFd[0][1] = fPipeFd[1][0] = fPipeFd[1][1] = -1;

    if (::pipe(fPipeFd[0]) == -1) {
        LOG_STDERR_ERRNO("Could not create plugin->helper pipe");
        return;
    }

    if (::pipe(fPipeFd[1]) == -1) {
        LOG_STDERR_ERRNO("Could not create helper->plugin pipe");
        return;
    }

    ipc_conf_t conf;
    conf.fd_r = fPipeFd[1][0];
    conf.fd_w = fPipeFd[0][1];
    fIpc = ipc_init(&conf);

    fIpcThread = new IpcReadThread(*this);
    fIpcThread->startThread();

    char rfd[10];
    ::sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    ::sprintf(wfd, "%d", fPipeFd[1][1]);
    // BIN_BASENAME is defined in Makefile
    String helperPath = platform::getBinaryDirectoryPath() + "/" XSTR(BIN_BASENAME) "_helper";

    const char *argv[] = {helperPath, rfd, wfd, 0};
    int status = ::posix_spawn(&fPid, helperPath, 0, 0, (char* const*)argv, environ);
    if (status != 0) {
        LOG_STDERR_ERRNO("Could not spawn helper subprocess");
        return;
    }

    helper_size_t size = {getSize().getWidth(), getSize().getHeight()};
    ipcWrite(OPC_RESIZE, &size, sizeof(size));

    String url = getContentUrl();
    const char *cUrl = static_cast<const char *>(url);
    ipcWrite(OPC_NAVIGATE, cUrl, ::strlen(cUrl) + 1);
}

ExternalGtkWebViewUI::~ExternalGtkWebViewUI()
{
    if (fPid != -1) {
        if (kill(fPid, SIGTERM) == 0) {
            int stat;
            ::waitpid(fPid, &stat, 0);
        } else {
            LOG_STDERR_ERRNO("Could not terminate helper subprocess");
        }
        fPid = -1;
    }

    if (fIpcThread != nullptr) {
        fIpcThread->stopThread(-1);
        fIpcThread = nullptr;
    }

    if (fIpc != nullptr) {
        ipc_destroy(fIpc);
        fIpc = nullptr;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if ((fPipeFd[i][j] != -1) && (close(fPipeFd[i][j]) == -1)) {
                LOG_STDERR_ERRNO("Could not close pipe");
            }
            fPipeFd[i][j] = -1;
        }
    }
}

void ExternalGtkWebViewUI::parameterChanged(uint32_t index, float value)
{
    helper_parameter_t param = {index, value};
    ipcWrite(OPC_SET_PARAMETER, &param, sizeof(param));
}

void ExternalGtkWebViewUI::reparent(uintptr_t windowId)
{
    ipcWrite(OPC_REPARENT, &windowId, sizeof(windowId));
}

void ExternalGtkWebViewUI::onResize(const ResizeEvent& ev)
{
    WebUI::onResize(ev);
    helper_size_t size = {ev.size.getWidth(), ev.size.getHeight()};
    ipcWrite(OPC_RESIZE, &size, sizeof(size));
}

int ExternalGtkWebViewUI::ipcWrite(helper_opcode_t opcode, const void *payload, int payloadSize)
{
    tlv_t packet;
    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &packet)) == -1) {
        LOG_STDERR_ERRNO("Could not write to IPC channel");
    }

    return retval;
}

void ExternalGtkWebViewUI::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<helper_opcode_t>(packet.t)) {
        case OPC_SET_PARAMETER: {
            const helper_parameter_t *param = static_cast<const helper_parameter_t *>(packet.v);
            setParameterValue(param->index, param->value);
            break;
        }
        default:
            break;
    }
}

IpcReadThread::IpcReadThread(ExternalGtkWebViewUI& view) : Thread("ipc_read"), fView(view) {}

void IpcReadThread::run()
{
    int fd = ipc_get_config(fView.ipc())->fd_r;
    fd_set rfds;
    struct timeval tv;
    tlv_t packet;

    while (true) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        int retval = ::select(fd + 1, &rfds, 0, 0, &tv);

        if (retval == -1) {
            LOG_STDERR_ERRNO("Failed select() on IPC channel");
            break;
        }

        if (shouldThreadExit()) {
            break;
        }

        if (retval == 0) {
            continue;   // select() timeout
        }

        if (ipc_read(fView.ipc(), &packet) == -1) {
            LOG_STDERR_ERRNO("Could not read from IPC channel");
            break;
        }

        fView.ipcReadCallback(packet);
    }
}
