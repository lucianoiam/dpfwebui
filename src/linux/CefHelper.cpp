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

#include "CefHelper.hpp"

#include <sstream>

#include <sys/select.h>
#include <X11/Xutil.h>

#include "macro.h"

static int XErrorHandlerImpl(Display* display, XErrorEvent* event);
static int XIOErrorHandlerImpl(Display* display);

// Entry point function for all processes
int main(int argc, char* argv[])
{
    // Provide CEF with command-line arguments
    CefMainArgs args(argc, argv);

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int code = CefExecuteProcess(args, nullptr, nullptr);
    if (code >= 0) {
        // The sub-process has completed so return here
        return code;
    }

    ipc_conf_t conf;

    if (argc < 3) {
        HIPHOP_LOG_STDERR("Invalid argument count");
        return -1;
    }

    if ((sscanf(argv[1], "%d", &conf.fd_r) == 0) || (sscanf(argv[2], "%d", &conf.fd_w) == 0)) {
        HIPHOP_LOG_STDERR("Invalid file descriptor");
        return -1;
    }

    ipc_t* ipc = ipc_init(&conf);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    CefSettings settings;
    //settings.no_sandbox = true;
    settings.chrome_runtime = false;

    // CefHelper implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefHelper* app = new CefHelper(ipc);

    // Initialize CEF for the browser process
    CefInitialize(args, settings, app, nullptr);

    app->runMainLoop();
    delete app;

    // fBrowser must be deleted before calling CefShutdown() otherwise it hangs
    CefShutdown();

    ipc_destroy(ipc);

    return 0;
}

CefHelper::CefHelper(ipc_t* ipc)
    : fRun(false)
    , fIpc(ipc)
    , fDisplay(0)
    , fContainer(0)
{
    fDisplay = XOpenDisplay(NULL);

    if (fDisplay == NULL) {
        HIPHOP_LOG_STDERR("Cannot open display");
    }
}

CefHelper::~CefHelper()
{
    if (fContainer != 0) {
        XDestroyWindow(fDisplay, fContainer);
    }

    if (fDisplay != 0) {
        XCloseDisplay(fDisplay);
    }
}

void CefHelper::runMainLoop()
{
    int fd = ipc_get_config(fIpc)->fd_r;
    fd_set rfds;
    struct timeval tv;
    tlv_t packet;

    fRun = true;
    
    while (fRun) {
        CefDoMessageLoopWork();

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = tv.tv_usec = 0; // poll

        int retval = select(fd + 1, &rfds, 0, 0, &tv);

        if (retval == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Failed select() on IPC channel");
            fRun = false;
            continue;
        }

        if (retval == 0) {
            continue; // no fd ready
        }

        if (ipc_read(fIpc, &packet) == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Could not read from IPC channel");
            fRun = false;
            continue;
        }

        dispatch(&packet);
    }
}

void CefHelper::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine)
{
    commandLine->AppendSwitch("disable-extensions");
}

void CefHelper::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                            TransitionType transitionType)
{
    for (std::vector<CefString>::iterator it = fInjectedScripts.begin(); it != fInjectedScripts.end(); ++it) {
        frame->ExecuteJavaScript(*it, "", 0);
    }
}

void CefHelper::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          int httpStatusCode)
{
    XMapWindow(fDisplay, fContainer);
    XSync(fDisplay, False);

    // TODO
}

void CefHelper::dispatch(const tlv_t* packet)
{
    switch (static_cast<msg_opcode_t>(packet->t)) {
        case OP_REALIZE:
            realize((const msg_win_cfg_t *)packet->v);
            break;

        case OP_NAVIGATE: {
            const char* url = static_cast<const char*>(packet->v);
            fBrowser->GetMainFrame()->LoadURL(url);
            break;
        }

        case OP_RUN_SCRIPT:
            fBrowser->GetMainFrame()->ExecuteJavaScript(static_cast<const char*>(packet->v), "", 0);
            break;

        case OP_INJECT_SCRIPT:
            fInjectedScripts.push_back(static_cast<const char*>(packet->v));
            break;

        case OP_SET_SIZE: {
            // TODO - untested
            /*const msg_win_size_t *size = (const msg_win_size_t *)packet->v;
            ::Display* display = cef_get_xdisplay();
            ::Window window = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
            XResizeWindow(display, window, size->width, size->height);
            XSync(display, False);*/
            break;
        }

        case OP_SET_KEYBOARD_FOCUS:
            // TODO
            break;

        case OP_TERMINATE:
            fRun = false;
            break;

        default:
            break;
    }
}

void CefHelper::realize(const msg_win_cfg_t *config)
{
    // Top view is needed to ensure 24-bit colormap otherwise CreateBrowserSync()
    // will fail producing multiple Xlib errors. This can only be reproduced on
    // REAPER when trying to open the plugin interface by clicking the UI button.

    XVisualInfo vinfo;
    XMatchVisualInfo(fDisplay, DefaultScreen(fDisplay), 24, TrueColor, &vinfo);

    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(fDisplay, XDefaultRootWindow(fDisplay),
                                     vinfo.visual, AllocNone);

    fContainer = XCreateWindow(fDisplay, static_cast<::Window>(config->parent),
                               0, 0, config->size.width, config->size.height, 0,
                               vinfo.depth, CopyFromParent, vinfo.visual,
                               CWColormap, &attrs);
    XSync(fDisplay, false);

    CefBrowserSettings settings;

    // TODO
    //settings.log_severity = DISABLE;

    CefWindowInfo windowInfo;
    windowInfo.parent_window = fContainer;
    windowInfo.width = config->size.width;
    windowInfo.height = config->size.height;

    fBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings,
        nullptr, nullptr);
}

static int XErrorHandlerImpl(Display* display, XErrorEvent* event)
{
    std::stringstream ss;

    ss << "X error received: "
       << "type " << event->type << ", "
       << "serial " << event->serial << ", "
       << "error_code " << static_cast<int>(event->error_code) << ", "
       << "request_code " << static_cast<int>(event->request_code) << ", "
       << "minor_code " << static_cast<int>(event->minor_code);

    HIPHOP_LOG_STDERR_COLOR(ss.str().c_str());
    
    return 0;
}

static int XIOErrorHandlerImpl(Display* display)
{
    return 0;
}
