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
#include <sys/select.h>
#include <sys/wait.h>

#include "Path.hpp"
#include "macro.h"

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

extern char **environ;

/*
  Need to launch a child process hosting the web view

  - CEF lifecycle and usage of globals is not compatible with plugins
    https://bitbucket.org/chromiumembedded/cef/issues/421

  - WebKitGTK is a no-go because linking plugins to UI toolkits is problematic
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

    fPipeFd[0][0] = fPipeFd[0][1] = fPipeFd[1][0] = fPipeFd[1][1] = -1;

    if (pipe(fPipeFd[0]) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not create parent->helper pipe");
        return;
    }

    if (pipe(fPipeFd[1]) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not create helper->parent pipe");
        return;
    }

    ipc_conf_t conf;
    conf.fd_r = fPipeFd[1][0];
    conf.fd_w = fPipeFd[0][1];

    fIpc = ipc_init(&conf);
    fIpcThread = new IpcReadThread(*this);
    fIpcThread->startThread();

    // BIN_BASENAME is defined in Makefile
    char rfd[10];
    sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    sprintf(wfd, "%d", fPipeFd[1][1]);
    
    String helperPath = path::getLibraryPath() + "/ui-helper";
    const char *argv[] = { helperPath, rfd, wfd, 0 };

    int status = posix_spawn(&fPid, helperPath, 0, 0, (char* const*)argv, environ);

    if (status != 0) {
        HIPHOP_LOG_STDERR_ERRNO("Could not spawn helper subprocess");
    }
}

ChildProcessWebView::~ChildProcessWebView()
{
    if (fPid != -1) {
        ipcWrite(OP_TERMINATE, 0, 0);

        int stat;
        waitpid(fPid, &stat, 0);

        fPid = -1;
    }

    if (fIpcThread != 0) {
        fIpcThread->stopThread(-1);
        fIpcThread = 0;
    }

    if (fIpc != 0) {
        ipc_destroy(fIpc);
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
    ipcWrite(OP_REALIZE, &config, sizeof(config));

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
}

void ChildProcessWebView::navigate(String& url)
{
    ipcWriteString(OP_NAVIGATE, url);
}

void ChildProcessWebView::runScript(String& source)
{
    ipcWriteString(OP_RUN_SCRIPT, source);
}

void ChildProcessWebView::injectScript(String& source)
{
    ipcWriteString(OP_INJECT_SCRIPT, source);
}

void ChildProcessWebView::onSize(uint width, uint height)
{
    if (fBackground == 0) {
        return;
    }

    XResizeWindow(fDisplay, fBackground, width, height);
    XSync(fDisplay, False);

    msg_win_size_t sizePkt = { width, height };
    ipcWrite(OP_SET_SIZE, &sizePkt, sizeof(sizePkt));
}

void ChildProcessWebView::onKeyboardFocus(bool focus)
{
    char val = focus ? 1 : 0;
    ipcWrite(OP_SET_KEYBOARD_FOCUS, &val, sizeof(val));
}

int ChildProcessWebView::ipcWriteString(msg_opcode_t opcode, String str) const
{
    const char *cStr = static_cast<const char *>(str);
    return ipcWrite(opcode, cStr, strlen(cStr) + 1);
}

int ChildProcessWebView::ipcWrite(msg_opcode_t opcode, const void *payload, int payloadSize) const
{
    tlv_t packet;
    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &packet)) == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not write to IPC channel");
    }

    return retval;
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

IpcReadThread::IpcReadThread(ChildProcessWebView& view)
    : Thread("ipc_read_" XSTR(HIPHOP_PROJECT_ID_HASH))
    , fView(view)
{}

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

        int retval = select(fd + 1, &rfds, 0, 0, &tv);

        if (retval == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Failed select() on IPC channel");
            break;
        }

        if (shouldThreadExit()) {
            break;
        }

        if (retval == 0) {
            continue; // timeout
        }

        if (ipc_read(fView.ipc(), &packet) == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Could not read from IPC channel");
            break;
        }

        fView.ipcReadCallback(packet);
    }
}
