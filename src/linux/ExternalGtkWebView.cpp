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

#include "ExternalGtkWebView.hpp"

#include <cstdio>
#include <cstring>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>

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
    if (!isRunning()) {
        spawn();
    }

    send(OPCODE_REPARENT, &parentWindowId, sizeof(parentWindowId));
}

int ExternalGtkWebView::spawn()
{
    if (isRunning()) {
        return -1;
    }

    if ((pipe(fPipeFd[0]) == -1) /*plugin->helper*/ || (pipe(fPipeFd[1]) == -1) /*p<-h*/) {
        perror("Could not create pipe");
        return -1;
    }

    fIpc = ipc_init(fPipeFd[1][0], fPipeFd[0][1]);

    fIpcThread = new IpcReadThread(*this);
    fIpcThread->startThread();

    char rfd[10];
    ::sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    ::sprintf(wfd, "%d", fPipeFd[1][1]);
    const char *argv[] = {"d_dpf_webui_helper", rfd, wfd, NULL};
    const char* fixmeHardcodedPath = "/home/user/dpf-webui/bin/d_dpf_webui_helper";
    
    int status = posix_spawn(&fPid, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);

    if (status != 0) {
        perror("Could not spawn subprocess");
        return -1;
    }

    String url = getContentUrl();
    const char *cUrl = static_cast<const char *>(url);
    send(OPCODE_NAVIGATE, cUrl, ::strlen(cUrl) + 1);

    return 0;
}

void ExternalGtkWebView::terminate()
{
    if (fPid != -1) {
        if (send(OPCODE_TERMINATE, NULL, 0) == -1) {
            kill(fPid, SIGKILL);
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
                perror("Could not close pipe");
            }
            fPipeFd[i][j] = -1;
        }
    }
}

int ExternalGtkWebView::send(char opcode, const void *payload, int size)
{
    ipc_msg_t msg;
    msg.opcode = opcode;
    msg.payload_sz = size;
    msg.payload = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &msg)) == -1) {
        perror("Could not write pipe");
    }

    return retval;
}

void ExternalGtkWebView::readCallback(const ipc_msg_t& message) const
{
    // TODO
    fprintf(stderr, "Plugin got message opcode = %d payload_sz = %d\n",
        message.opcode, message.payload_sz);
}

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

        if (shouldThreadExit()) {
            break;
        }

        if (retval == 0) {
            continue;   // select() timeout
        }

        if (ipc_read(fView.ipc(), &message) == -1) {
            perror("Could not read pipe");
            break;
        }

        fView.readCallback(message);
    }
}
