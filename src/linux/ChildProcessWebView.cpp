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

#include "ChildProcessWebView.hpp"

#include <cstdio>
#include <libgen.h>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>

#include "Path.hpp"
#include "macro.h"

extern char **environ;

/*
  Need to launch a child process hosting the web view

  - CEF lifecycle and usage of globals is not compatible with plugins
    https://bitbucket.org/chromiumembedded/cef/issues/421

  - WebKitGTK is a no-go because linking plugins to UI toolkits is a bad idea
*/

USE_NAMESPACE_DISTRHO

ChildProcessWebView::ChildProcessWebView()
    : fDisplay(0)
    , fBackground(0)
    , fPid(-1)
    , fIpc(0)
    , fIpcThread(0)
{
    fDisplay = XOpenDisplay(0);

    fPipeFd[0][0] = -1;
    fPipeFd[0][1] = -1;

    if (pipe(fPipeFd[0]) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not create host->helper pipe");
        return;
    }

    fPipeFd[1][0] = -1;
    fPipeFd[1][1] = -1;

    if (pipe(fPipeFd[1]) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not create helper->host pipe");
        return;
    }

    fIpc = new IpcWrapper(fPipeFd[1][0], fPipeFd[0][1], 100/*ms*/);
    fIpcThread = new IpcReadThread(fIpc,
        std::bind(&ChildProcessWebView::ipcReadCallback, this, std::placeholders::_1));
    fIpcThread->startThread();

    char rfd[10];
    sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    sprintf(wfd, "%d", fPipeFd[1][1]);
    
    String libPath = path::getLibraryPath();
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addchdir_np(&fa, libPath);

    String helperPath = libPath + "/ui-helper";
    const char *argv[] = { helperPath, rfd, wfd, 0 };

    int status = posix_spawnp(&fPid, helperPath, &fa, 0, (char* const*)argv, environ);

    if (status != 0) {
        HIPHOP_LOG_STDERR_ERRNO("Could not spawn helper child process");
        return;
    }

    injectDefaultScripts();
}

ChildProcessWebView::~ChildProcessWebView()
{
    if (fPid != -1) {
        fIpc->write(OP_TERMINATE);
#ifdef LXHELPER_SIGTERM
        kill(fPid, SIGTERM);
#endif
        int stat;
        waitpid(fPid, &stat, 0);

        fPid = -1;
    }

    if (fIpcThread != 0) {
        fIpcThread->stopThread(-1);
        fIpcThread = 0;
    }

    if (fIpc != 0) {
        delete fIpc;
        fIpc = 0;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if ((fPipeFd[i][j] != -1) && (close(fPipeFd[i][j]) == -1)) {
                HIPHOP_LOG_STDERR_ERRNO("Could not close pipe");
            }

            fPipeFd[i][j] = -1;
        }
    }

    if (fBackground != 0) {
        XDestroyWindow(fDisplay, fBackground);
    }

    XCloseDisplay(fDisplay);
}

void ChildProcessWebView::realize()
{
    ::Window parent = (::Window)getParent();
    unsigned long color = getBackgroundColor() >> 8;

    // The only reliable way to keep background color while window manager open
    // and close animations are performed is to paint the provided window. This
    // is needed for hosts that show floating windows like Carla and Bitwig and
    // at least true for the Gnome Shell window manager.
    XSetWindowBackground(fDisplay, parent, color);
    XClearWindow(fDisplay, parent);

    // A colored top view is also needed to avoid initial flicker on REAPER
    // because the child process takes non-zero time to start
    fBackground = XCreateSimpleWindow(fDisplay, parent, 0, 0, getWidth(), getHeight(), 0, 0, 0);
    XMapWindow(fDisplay, fBackground);
    XSetWindowBackground(fDisplay, fBackground, color);
    XClearWindow(fDisplay, fBackground);
    XSync(fDisplay, False);

    msg_win_cfg_t config;
    config.parent = static_cast<uintptr_t>(fBackground);
    config.color = color;
    config.size = { getWidth(), getHeight() };
    fIpc->write(OP_REALIZE, &config, sizeof(config));
}

void ChildProcessWebView::navigate(String& url)
{
    fIpc->write(OP_NAVIGATE, url);
}

void ChildProcessWebView::runScript(String& source)
{
    fIpc->write(OP_RUN_SCRIPT, source);
}

void ChildProcessWebView::injectScript(String& source)
{
    fIpc->write(OP_INJECT_SCRIPT, source);
}

void ChildProcessWebView::onSize(uint width, uint height)
{
    if (fBackground == 0) {
        return;
    }

    XResizeWindow(fDisplay, fBackground, width, height);
    XSync(fDisplay, False);

    msg_win_size_t sizePkt = { width, height };
    fIpc->write(OP_SET_SIZE, &sizePkt, sizeof(sizePkt));
}

void ChildProcessWebView::onKeyboardFocus(bool focus)
{
    char val = focus ? 1 : 0;
    fIpc->write(OP_SET_KEYBOARD_FOCUS, &val, sizeof(val));
}

void ChildProcessWebView::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<msg_opcode_t>(packet.t)) {
        case OP_HANDLE_LOAD_FINISHED: {
            handleLoadFinished();
            break;
        }
        case OP_HANDLE_SCRIPT_MESSAGE:
            handleHelperScriptMessage(static_cast<const char*>(packet.v), packet.l);
            break;

        default:
            break;
    }
}

void ChildProcessWebView::handleHelperScriptMessage(const char *payload, int payloadSize)
{
    // Should validate payload is never read past payloadSize 
    JsValueVector args;
    int offset = 0;

    while (offset < payloadSize) {
        const char *type = payload + offset;
        const char *value = type + 1;

        switch (*type) {
            case ARG_TYPE_FALSE:
                offset += 1;
                args.push_back(JsValue(false));
                break;

            case ARG_TYPE_TRUE:
                offset += 1;
                args.push_back(JsValue(true));
                break;

            case ARG_TYPE_DOUBLE:
                offset += 1 + sizeof(double);
                args.push_back(JsValue(*reinterpret_cast<const double *>(value)));
                break;

            case ARG_TYPE_STRING:
                offset += 1 /*type*/ + strlen(value) + 1 /*\0*/;
                args.push_back(JsValue(String(value)));
                break;

            default:
                offset += 1;
                args.push_back(JsValue()); // null
                break;
        }
    }

    handleScriptMessage(args);
}

IpcReadThread::IpcReadThread(IpcWrapper* ipc, IpcReadCallback callback)
    : Thread("ipc_read_" XSTR(HIPHOP_PROJECT_ID_HASH))
    , fIpc(ipc)
    , fCallback(callback)
{}

void IpcReadThread::run()
{
    tlv_t packet;

    while (!shouldThreadExit()) {
        if (fIpc->read(&packet) == 0) {
            fCallback(packet);
        }
    }
}
