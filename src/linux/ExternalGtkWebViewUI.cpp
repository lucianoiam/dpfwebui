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
#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "../log.h"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

extern char **environ;

static char _dummy; // for dladdr()

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

    if (pipe(fPipeFd[0]) == -1) {
        LOG_STDERR_ERRNO("Could not create plugin->helper pipe");
        return;
    }

    if (pipe(fPipeFd[1]) == -1) {
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
    String helperPath = getSharedLibraryDirectoryPath() + "/" XSTR(/* see Makefile */ BIN_BASENAME) "_helper";

    const char *argv[] = {helperPath, rfd, wfd, NULL};
    int status = posix_spawn(&fPid, helperPath, NULL, NULL, (char* const*)argv, environ);
    if (status != 0) {
        LOG_STDERR_ERRNO("Could not spawn helper subprocess");
        return;
    }

    String url = getContentUrl();
    const char *cUrl = static_cast<const char *>(url);
    ipcWrite(OPC_NAVIGATE, cUrl, ::strlen(cUrl) + 1);
}

ExternalGtkWebViewUI::~ExternalGtkWebViewUI()
{
    if (fPid != -1) {
        if (kill(fPid, SIGTERM) == 0) {
            int stat;
            waitpid(fPid, &stat, 0);
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
    parameter_t param = {index, value};
    ipcWrite(OPC_SET_PARAMETER, &param, sizeof(param));
}

void ExternalGtkWebViewUI::reparent(uintptr_t windowId)
{
    ipcWrite(OPC_REPARENT, &windowId, sizeof(windowId));
}

String ExternalGtkWebViewUI::getSharedLibraryDirectoryPath()
{
    Dl_info dl_info;
    if (dladdr((void *)&_dummy, &dl_info) == 0) {
        LOG_STDERR("Failed dladdr() call");
        return String();
    }
    char path[::strlen(dl_info.dli_fname) + 1];
    ::strcpy(path, dl_info.dli_fname);
    return String(dirname(path));
}

int ExternalGtkWebViewUI::ipcWrite(opcode_t opcode, const void *payload, int payloadSize)
{
    tlv_t packet;
    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &packet)) == -1) {
        LOG_STDERR_ERRNO_INT("Failed ipc_write() for opcode", opcode);
    }

    return retval;
}

void ExternalGtkWebViewUI::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<opcode_t>(packet.t)) {
        case OPC_SET_PARAMETER: {
            const parameter_t *param = static_cast<const parameter_t *>(packet.v);
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
        int retval = select(fd + 1, &rfds, NULL, NULL, &tv);

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
            LOG_STDERR_ERRNO("Failed ipc_read()");
            break;
        }

        fView.ipcReadCallback(packet);
    }
}
