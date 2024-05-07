/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#include "ChildProcessWebView.hpp"

#include <cstdio>
#include <errno.h>
#include <libgen.h>
#include <spawn.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>

#include "extra/macro.h"
#include "extra/Path.hpp"

extern char **environ;

/*
    On Linux a child process hosts the web view to workaround these issues:
  
  - WebKitGTK needs GTK and linking plugins to UI toolkits is a bad idea
    http://lists.lv2plug.in/pipermail/devel-lv2plug.in/2016-March/001593.html
    https://www.mail-archive.com/gtk-list%40gnome.org/msg34952.html

  - CEF lifecycle and usage of globals is not compatible with plugins
    https://bitbucket.org/chromiumembedded/cef/issues/421

    There is a lengthy relevant discussion on the topic here
    https://gist.github.com/abique/4c1b9b40f3413f0df1591d2a7c760db4
*/

USE_NAMESPACE_DISTRHO

#define IF_CHANNEL_CLOSED_RETURN() if (fIpc == nullptr) return

ChildProcessWebView::ChildProcessWebView(String userAgentComponent)
    : fUserAgent(userAgentComponent)
    , fDisplay(0)
    , fBackground(0)
    , fPipeFd {{-1, -1}, {-1, -1}}
    , fPid(-1)
    , fIpc(nullptr)
    , fIpcThread(nullptr)
    , fDevicePixelRatio(0)
{
    fDisplay = XOpenDisplay(0);

    if (fDisplay == 0) {
        d_stderr("Could not open display");
        cleanup();
        return;
    }

    if (pipe(fPipeFd[0]) == -1) {
        d_stderr("Could not create host->helper pipe - %s", strerror(errno));
        cleanup();
        return;
    }

    if (pipe(fPipeFd[1]) == -1) {
        d_stderr("Could not create helper->host pipe - %s", strerror(errno));
        cleanup();
        return;
    }

    fIpc = new IpcChannel(fPipeFd[1][0], fPipeFd[0][1], 100/*read timeout ms*/);
    fIpcThread = new IpcReadThread(fIpc,
        std::bind(&ChildProcessWebView::ipcReadCallback, this, std::placeholders::_1));
    fIpcThread->startThread();

    char rfd[10];
    std::sprintf(rfd, "%d", fPipeFd[0][0]);
    char wfd[10];
    std::sprintf(wfd, "%d", fPipeFd[1][1]);
    
    String libPath = Path::getPluginLibrary();
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addchdir_np(&fa, libPath);

    String helperPath = libPath + "/ui-helper";
    const char *argv[] = {helperPath, rfd, wfd, nullptr};

    const int status = posix_spawnp(&fPid, helperPath, &fa, 0, const_cast<char* const*>(argv),
                                    environ);

    if (status != 0) {
        d_stderr("Could not spawn helper child process - %s", strerror(errno));
        cleanup();
        return;
    }

    // Wait for init up to 3s
    for (int i = 0; (i < 300) && (fDevicePixelRatio == 0); i++) {
        usleep(10000L); // 10ms
    }

    if (fDevicePixelRatio == 0) {
        d_stderr("Timeout waiting for UI helper init - %s", strerror(errno));
        cleanup();
        return;
    }

    injectHostObjectScripts();

    // No drag and drop for GTK and CEF
    setEnvironmentBool("noDragAndDrop", true);

#if defined(DPF_WEBUI_LINUX_WEBVIEW_GTK)
# if DPF_WEBUI_LINUX_GTK_WEBVIEW_FAKE_VIEWPORT
    // CSS media queries for screen dimensions and CSS viewport units
    // (vw/vh/vmin/vmax) are not reliable when the UI is allowed to resize
    setEnvironmentBool("fakeViewport", true);
# endif
    // <input type="range"> elements do not react to touches
    setEnvironmentBool("noRangeInputTouch", true);
    // Overwrite 1.0 with value from device_pixel_ratio()
    String js = "window.devicePixelRatio=" + String(fDevicePixelRatio) + ";";
    injectScript(js);
#endif

    fIpc->write(OP_INJECT_SHIMS);
}

ChildProcessWebView::~ChildProcessWebView()
{
    cleanup();
}

float ChildProcessWebView::getDevicePixelRatio()
{
    // Determined by child processes
    return fDevicePixelRatio;
}

void ChildProcessWebView::realize()
{
    IF_CHANNEL_CLOSED_RETURN();

    const ::Window parent = (::Window)getParent();
    const unsigned long color = getBackgroundColor() >> 8;
    const uint width = getWidth();
    const uint height = getHeight();

    // The only reliable way to keep background color while window manager open
    // and close animations are performed is to paint the provided window. This
    // is needed for hosts that show floating windows like Carla and Bitwig and
    // at least true for the Gnome Shell window manager.
    XSetWindowBackground(fDisplay, parent, color);
    XClearWindow(fDisplay, parent);

    // A colored top view is also needed to avoid initial flicker on REAPER
    // because the child process takes non-zero time to start
    fBackground = XCreateSimpleWindow(fDisplay, parent, 0, 0, width, height,
                                        0, 0, 0);
    XMapWindow(fDisplay, fBackground);
    XSetWindowBackground(fDisplay, fBackground, color);
    XClearWindow(fDisplay, fBackground);
    XSync(fDisplay, False);

    msg_view_cfg_t config = {};
    config.parent = static_cast<uintptr_t>(fBackground);
    config.color = color;
    config.size = { width, height };
    std::strncpy(config.userAgent, fUserAgent.buffer(), sizeof(config.userAgent) - 1);
    fIpc->write(OP_REALIZE, &config, sizeof(config));
}

void ChildProcessWebView::navigate(String& url)
{
    IF_CHANNEL_CLOSED_RETURN();

    fIpc->write(OP_NAVIGATE, url);
}

void ChildProcessWebView::runScript(String& source)
{
    IF_CHANNEL_CLOSED_RETURN();

    fIpc->write(OP_RUN_SCRIPT, source);
}

void ChildProcessWebView::injectScript(String& source)
{
    IF_CHANNEL_CLOSED_RETURN();

    fIpc->write(OP_INJECT_SCRIPT, source);
}

void ChildProcessWebView::onSize(uint width, uint height)
{
    IF_CHANNEL_CLOSED_RETURN();

    if (fBackground != 0) {
        XResizeWindow(fDisplay, fBackground, width, height);
        XSync(fDisplay, False);
    }

    const msg_view_size_t sizePkt = { width, height };
    fIpc->write(OP_SET_SIZE, &sizePkt, sizeof(sizePkt));
}

void ChildProcessWebView::onKeyboardFocus(bool focus)
{
    IF_CHANNEL_CLOSED_RETURN();

    const char val = focus ? 1 : 0;
    fIpc->write(OP_SET_KEYBOARD_FOCUS, &val, sizeof(val));
}

void ChildProcessWebView::ipcReadCallback(const tlv_t& packet)
{
    switch (static_cast<msg_opcode_t>(packet.t)) {
        case OP_HANDLE_INIT:
            handleInit(*static_cast<const float*>(packet.v));
            break;
        case OP_HANDLE_LOAD_FINISHED:
            handleLoadFinished();
            break;
        case OP_HANDLE_SCRIPT_MESSAGE:
            handleHelperScriptMessage(static_cast<const char*>(packet.v), packet.l);
            break;
        default:
            break;
    }
}

void ChildProcessWebView::handleInit(float devicePixelRatio)
{
    fDevicePixelRatio = devicePixelRatio;
}

void ChildProcessWebView::handleHelperScriptMessage(const char *payloadBytes,
                                                    int payloadSize)
{
    // Should validate payload is never read past payloadSize 
    Variant payload = Variant::createArray();
    int offset = 0;

    while (offset < payloadSize) {
        const char *type = payloadBytes + offset;
        const char *value = type + 1;

        switch (*type) {
            case ARG_TYPE_FALSE:
                offset += 1;
                payload.pushArrayItem(false);
                break;
            case ARG_TYPE_TRUE:
                offset += 1;
                payload.pushArrayItem(true);
                break;
            case ARG_TYPE_DOUBLE:
                offset += 1 + sizeof(double);
                payload.pushArrayItem(*reinterpret_cast<const double *>(value));
                break;
            case ARG_TYPE_STRING:
                offset += 1 /*type*/ + strlen(value) + 1 /*\0*/;
                payload.pushArrayItem(static_cast<const char*>(value));
                break;
            default:
                offset += 1;
                payload.pushArrayItem(Variant()); // null
                break;
        }
    }

    handleScriptMessage(payload);
}

void ChildProcessWebView::cleanup()
{
    if (fIpc != 0) {
        if (fPid != -1) {
            fIpc->write(OP_TERMINATE);
#if defined(DPF_WEBUI_LINUX_WEBVIEW_CEF)
            kill(fPid, SIGTERM);
#endif
            int stat;
            waitpid(fPid, &stat, 0);
            fPid = -1;
        }

        delete fIpc;
        fIpc = 0;
    }

    if (fIpcThread != 0) {
        fIpcThread->stopThread(-1);
        fIpcThread = 0;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if ((fPipeFd[i][j] != -1) && (close(fPipeFd[i][j]) == -1)) {
                d_stderr("Could not close pipe - %s", strerror(errno));
            }

            fPipeFd[i][j] = -1;
        }
    }

    if (fBackground != 0) {
        XDestroyWindow(fDisplay, fBackground);
        fBackground = 0;
    }

    if (fDisplay != 0) {
        XCloseDisplay(fDisplay);
        fDisplay = 0;
    }
}

IpcReadThread::IpcReadThread(IpcChannel* ipc, IpcReadCallback callback)
    : Thread("ipc_read_" XSTR(DPF_WEBUI_PROJECT_ID_HASH))
    , fIpc(ipc)
    , fCallback(callback)
{}

void IpcReadThread::run()
{
    tlv_t packet;

    while (! shouldThreadExit()) {
        if (fIpc->read(&packet) == 0) {
            fCallback(packet);
        }
    }
}
