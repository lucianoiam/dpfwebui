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

#include "ExternalGtkWebWidget.hpp"

#include <cstdio>
#include <libgen.h>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "base/Platform.hpp"
#include "base/macro.h"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

// CSS touch-action based approach seems to be failing for WebKitGTK. Looks like a bug.
#define JS_DISABLE_PINCH_ZOOM_WORKAROUND "if (document.body.children.length > 0) document.body.children[0].addEventListener('touchstart', (ev) => { ev.preventDefault(); });"

extern char **environ;

USE_NAMESPACE_DISTRHO

ExternalGtkWebWidget::ExternalGtkWebWidget(Window& windowToMapTo)
    : AbstractWebWidget(windowToMapTo)
    , fPid(-1)
    , fIpc(nullptr)
    , fIpcThread(nullptr)
{
    fPipeFd[0][0] = fPipeFd[0][1] = fPipeFd[1][0] = fPipeFd[1][1] = -1;

    if (pipe(fPipeFd[0]) == -1) {
        DISTRHO_LOG_STDERR_ERRNO("Could not create parent->helper pipe");
        return;
    }

    if (pipe(fPipeFd[1]) == -1) {
        DISTRHO_LOG_STDERR_ERRNO("Could not create helper->parent pipe");
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
    
    char binPath[PATH_MAX];
    strcpy(binPath, platform::getBinaryPath());
    String helperPath = String(dirname(binPath)) + "/" XSTR(BIN_BASENAME) "_ui";
    const char *argv[] = {helperPath, rfd, wfd, 0};

    int status = posix_spawn(&fPid, helperPath, 0, 0, (char* const*)argv, environ);

    if (status != 0) {
        DISTRHO_LOG_STDERR_ERRNO("Could not spawn helper subprocess");
        return;
    }

    int windowId = static_cast<int>(windowToMapTo.getNativeWindowHandle());
    ipcWrite(OPC_SET_PARENT, &windowId, sizeof(windowId));

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
}

ExternalGtkWebWidget::~ExternalGtkWebWidget()
{
    if (fPid != -1) {
        if (kill(fPid, SIGTERM) == 0) {
            int stat;
            waitpid(fPid, &stat, 0);
        } else {
            DISTRHO_LOG_STDERR_ERRNO("Could not terminate helper subprocess");
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
                DISTRHO_LOG_STDERR_ERRNO("Could not close pipe");
            }

            fPipeFd[i][j] = -1;
        }
    }
}

void ExternalGtkWebWidget::onResize(const ResizeEvent& ev)
{
    helper_size_t sizePkt = {ev.size.getWidth(), ev.size.getHeight()};
    ipcWrite(OPC_SET_SIZE, &sizePkt, sizeof(sizePkt));
}

bool ExternalGtkWebWidget::onKeyboard(const KeyboardEvent& ev)
{
    // Some hosts like Bitwig prevent the web view from getting keyboard focus
    helper_key_t keyPkt = {static_cast<char>(ev.press), static_cast<unsigned>(ev.key),
        static_cast<unsigned>(ev.keycode)};
    ipcWrite(OPC_KEY_EVENT, &keyPkt, sizeof(keyPkt));
    return isGrabKeyboardInput(); // true = stop propagation
}

void ExternalGtkWebWidget::setBackgroundColor(uint32_t rgba)
{
    ipcWrite(OPC_SET_BACKGROUND_COLOR, &rgba, sizeof(uint32_t));
}

void ExternalGtkWebWidget::navigate(String& url)
{
    ipcWriteString(OPC_NAVIGATE, url);
}

void ExternalGtkWebWidget::runScript(String& source)
{
    ipcWriteString(OPC_RUN_SCRIPT, source);
}

void ExternalGtkWebWidget::injectScript(String& source)
{
    ipcWriteString(OPC_INJECT_SCRIPT, source);
}

int ExternalGtkWebWidget::ipcWriteString(helper_opcode_t opcode, String str) const
{
    const char *cStr = static_cast<const char *>(str);
    return ipcWrite(opcode, cStr, strlen(cStr) + 1);
}

int ExternalGtkWebWidget::ipcWrite(helper_opcode_t opcode, const void *payload, int payloadSize) const
{
    tlv_t packet;
    packet.t = static_cast<short>(opcode);
    packet.l = payloadSize;
    packet.v = payload;

    int retval;

    if ((retval = ipc_write(fIpc, &packet)) == -1) {
        DISTRHO_LOG_STDERR_ERRNO("Could not write to IPC channel");
    }

    return retval;
}

void ExternalGtkWebWidget::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<helper_opcode_t>(packet.t)) {
        case OPC_HANDLE_LOAD_FINISHED: {
            String js = String(JS_DISABLE_PINCH_ZOOM_WORKAROUND);
            runScript(js);
            handleLoadFinished();
            break;
        }
        case OPC_HANDLE_SCRIPT_MESSAGE:
            handleHelperScriptMessage(static_cast<const char*>(packet.v), packet.l);
            break;

        default:
            break;
    }
}

void ExternalGtkWebWidget::handleHelperScriptMessage(const char *payload, int payloadSize)
{
    // Should validate payload is never read past payloadSize 
    ScriptValueVector args;
    int offset = 0;

    while (offset < payloadSize) {
        const char *type = payload + offset;
        const char *value = type + 1;

        switch (*type) {
            case ARG_TYPE_FALSE:
                offset += 1;
                args.push_back(ScriptValue(false));
                break;

            case ARG_TYPE_TRUE:
                offset += 1;
                args.push_back(ScriptValue(true));
                break;

            case ARG_TYPE_DOUBLE:
                offset += 1 + sizeof(double);
                args.push_back(ScriptValue(*reinterpret_cast<const double *>(value)));
                break;

            case ARG_TYPE_STRING:
                offset += 1 /*type*/ + strlen(value) + 1 /*\0*/;
                args.push_back(ScriptValue(String(value)));
                break;

            default:
                offset += 1;
                args.push_back(ScriptValue()); // null
                break;
        }
    }

    handleScriptMessage(args);
}

IpcReadThread::IpcReadThread(ExternalGtkWebWidget& view)
    : Thread("ipc_read")
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
            DISTRHO_LOG_STDERR_ERRNO("Failed select() on IPC channel");
            break;
        }

        if (shouldThreadExit()) {
            break;
        }

        if (retval == 0) {
            continue; // timeout
        }

        if (ipc_read(fView.ipc(), &packet) == -1) {
            DISTRHO_LOG_STDERR_ERRNO("Could not read from IPC channel");
            break;
        }

        fView.ipcReadCallback(packet);
    }
}
