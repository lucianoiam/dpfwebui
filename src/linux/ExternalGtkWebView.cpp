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

#include "ExternalGtkWebView.hpp"

#include <cstdio>
#include <cstring>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "helper.h"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

extern char **environ;

USE_NAMESPACE_DISTRHO

ExternalGtkWebView::ExternalGtkWebView()
    : fPid(-1)
    , fIpc(nullptr)
    , fIpcThread(nullptr)
{
    fPipeFd[0][0] = fPipeFd[0][1] = fPipeFd[1][0] = fPipeFd[1][1] = -1;
}

ExternalGtkWebView::~ExternalGtkWebView()
{
    terminate();
}

void ExternalGtkWebView::reparent(uintptr_t parentWindowId)
{
    if (!isRunning() && (spawn() == -1)) {
        return;
    }

    ipcWrite(OPCODE_REPARENT, &parentWindowId, sizeof(parentWindowId));
}

int ExternalGtkWebView::spawn()
{
    if (pipe(fPipeFd[0]) == -1) {
        LOG_STDERR_ERRNO("Could not create plugin->helper pipe");
        return -1;
    }

    if (pipe(fPipeFd[1]) == -1) {
        LOG_STDERR_ERRNO("Could not create helper->plugin pipe");
        return -1;
    }

    fIpc = ipc_init(fPipeFd[1][0], fPipeFd[0][1]);

    fIpcThread = new IpcReadThread(*this);
    fIpcThread->startThread();

    char rfd[10];
    ::sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    ::sprintf(wfd, "%d", fPipeFd[1][1]);
    const char* fixmeHardcodedPath = "/home/user/dpf-webui/bin/d_dpf_webui_helper";
    const char *argv[] = {fixmeHardcodedPath, rfd, wfd, NULL};

    int status = posix_spawn(&fPid, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);
    if (status != 0) {
        LOG_STDERR_ERRNO("Could not spawn helper subprocess");
        return -1;
    }

    String url = getContentUrl();
    const char *cUrl = static_cast<const char *>(url);
    ipcWrite(OPCODE_NAVIGATE, cUrl, ::strlen(cUrl) + 1);

    return 0;
}

void ExternalGtkWebView::terminate()
{
    if (fPid != -1) {
        if (ipcWrite(OPCODE_TERMINATE, NULL, 0) == -1) {
            if (kill(fPid, SIGTERM) == 0) {
                int stat;
                waitpid(fPid, &stat, 0); // avoid zombie
            } else {
                LOG_STDERR_ERRNO("Could not terminate helper subprocess");
            }
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

int ExternalGtkWebView::ipcWrite(char opcode, const void *payload, int size)
{
    ipc_msg_t msg;
    msg.opcode = opcode;
    msg.payload_sz = size;
    msg.payload = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &msg)) == -1) {
        LOG_STDERR_ERRNO_INT("Failed ipc_write(), opcode", opcode);
    }

    return retval;
}

void ExternalGtkWebView::ipcReadCallback(const ipc_msg_t& message) const
{
    ::fprintf(stderr, "FIXME - Message from helper: %s\n", (const char*)message.payload);
}

IpcReadThread::IpcReadThread(const ExternalGtkWebView& view)
    : Thread("ipc_read"), fView(view) {}

void IpcReadThread::run()
{
    int fd = ipc_get_read_fd(fView.ipc());
    fd_set rfds;
    struct timeval tv;
    ipc_msg_t message;

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

        if (ipc_read(fView.ipc(), &message) == -1) {
            LOG_STDERR_ERRNO("Failed ipc_read()");
            break;
        }

        fView.ipcReadCallback(message);
    }
}
