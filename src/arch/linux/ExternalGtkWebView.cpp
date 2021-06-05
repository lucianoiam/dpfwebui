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
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "base/Platform.hpp"
#include "base/log.h"
#include "base/macro.h"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

extern char **environ;

USE_NAMESPACE_DISTRHO

ExternalGtkWebView::ExternalGtkWebView(WebViewScriptMessageHandler& handler)
    : BaseWebView(handler)
    , fPid(-1)
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

    // BIN_BASENAME is defined in Makefile
    char rfd[10];
    ::sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    ::sprintf(wfd, "%d", fPipeFd[1][1]);
    String helperPath = platform::getBinaryDirectoryPath() + "/" XSTR(BIN_BASENAME) "_helper";
    const char *argv[] = {helperPath, rfd, wfd, 0};
    int status = ::posix_spawn(&fPid, helperPath, 0, 0, (char* const*)argv, environ);
    if (status != 0) {
        LOG_STDERR_ERRNO("Could not spawn helper subprocess");
        return;
    }

    createConsole();
}

ExternalGtkWebView::~ExternalGtkWebView()
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

void ExternalGtkWebView::reparent(uintptr_t windowId)
{
    ipcWrite(OPC_REPARENT, &windowId, sizeof(windowId));
}

void ExternalGtkWebView::resize(const Size<uint>& size)
{
    helper_size_t sizePkt = {size.getWidth(), size.getHeight()};
    ipcWrite(OPC_RESIZE, &sizePkt, sizeof(sizePkt));
}

void ExternalGtkWebView::navigate(String url)
{
    ipcWriteString(OPC_NAVIGATE, url);
}

void ExternalGtkWebView::runScript(String source)
{
    ipcWriteString(OPC_RUN_SCRIPT, source);
}

void ExternalGtkWebView::injectScript(String source)
{
    ipcWriteString(OPC_INJECT_SCRIPT, source);
}
    
int ExternalGtkWebView::ipcWriteString(helper_opcode_t opcode, String str)
{
    const char *cStr = static_cast<const char *>(str);
    return ipcWrite(opcode, cStr, ::strlen(cStr) + 1);
}

int ExternalGtkWebView::ipcWrite(helper_opcode_t opcode, const void *payload, int payloadSize)
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

void ExternalGtkWebView::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<helper_opcode_t>(packet.t)) {
        case OPC_HANDLE_LOAD_FINISHED:
            loadFinished();
            break;
        case OPC_HANDLE_SCRIPT_MESSAGE:
            handleHelperScriptMessage(static_cast<const char*>(packet.v), packet.l);
            break;
        default:
            break;
    }
}

void ExternalGtkWebView::handleHelperScriptMessage(const char *payload, int payloadSize)
{
    const char *name = payload;
    int offset = ::strlen(name) + 1;
    ScriptValue arg1, arg2;
    if (offset < payloadSize) {
        arg1 = nextScriptValue(payload, &offset);
        if (offset < payloadSize) {
            arg2 = nextScriptValue(payload, &offset);
        }
    }
    handleWebViewScriptMessage(String(name), arg1, arg2);
}

ScriptValue ExternalGtkWebView::nextScriptValue(const char *payload, int *offset)
{
    // Not validating bytesLeft...
    const char *typeAndValue = payload + *offset;
    switch (typeAndValue[0]) {
        case ARG_TYPE_NULL:
            *offset += 1;
            //::printf("Rx null\n");
            return ScriptValue();
            break;
        case ARG_TYPE_FALSE:
            //::printf("Rx false\n");
            *offset += 1;
            return ScriptValue(false);
            break;
        case ARG_TYPE_TRUE:
            //::printf("Rx true\n");
            *offset += 1;
            return ScriptValue(true);
            break;
        case ARG_TYPE_DOUBLE:
            //::printf("Rx %.2g\n", *reinterpret_cast<const double *>(typeAndValue + 1));
            *offset += 1 + sizeof(double);
            return ScriptValue(*reinterpret_cast<const double *>(typeAndValue + 1));
            break;
        case ARG_TYPE_STRING:
            //::printf("Rx (str) %s\n", typeAndValue + 1);
            *offset += 1 /*type*/ + ::strlen(typeAndValue + 1) + 1 /*null*/;
            return ScriptValue(String(typeAndValue + 1));
            break;
        default:
            return ScriptValue();
            break;
    }
}

IpcReadThread::IpcReadThread(ExternalGtkWebView& view)
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
        int retval = ::select(fd + 1, &rfds, 0, 0, &tv);
        if (retval == -1) {
            LOG_STDERR_ERRNO("Failed select() on IPC channel");
            break;
        }
        if (shouldThreadExit()) {
            break;
        }
        if (retval == 0) {
            continue; // timeout
        }
        if (ipc_read(fView.ipc(), &packet) == -1) {
            LOG_STDERR_ERRNO("Could not read from IPC channel");
            break;
        }
        fView.ipcReadCallback(packet);
    }
}
