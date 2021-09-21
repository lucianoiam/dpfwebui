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

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.hostPostMessage(args);"

static int XErrorHandlerImpl(Display* display, XErrorEvent* event);
static int XIOErrorHandlerImpl(Display* display);

// Entry point function for all processes
int main(int argc, char* argv[])
{
    CefMainArgs args(argc, argv);
    CefRefPtr<CefHelper> app = new CefHelper();

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int code = CefExecuteProcess(args, app.get(), nullptr);
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

    app->createIpc(conf);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    CefSettings settings;
    //settings.no_sandbox = true;
    settings.chrome_runtime = false;

    // Initialize CEF for the browser process
    CefInitialize(args, settings, app.get(), nullptr);

    app->runMainLoop();

    // fBrowser must be deleted before calling CefShutdown() otherwise it hangs
    app = nullptr;
    CefShutdown();

    return 0;
}

CefHelper::CefHelper()
    : fIpc(0)
    , fbRunMainLoop(false)
    , fbDisplay(0)
    , fbContainer(0)
{}

CefHelper::~CefHelper()
{
    if (fIpc != 0) {
        ipc_destroy(fIpc);
    }

    if (fbContainer != 0) {
        XDestroyWindow(fbDisplay, fbContainer);
    }

    if (fbDisplay != 0) {
        XCloseDisplay(fbDisplay);
    }
}

void CefHelper::createIpc(const ipc_conf_t& conf)
{
    fIpc = ipc_init(&conf);
}

void CefHelper::runMainLoop()
{
    fbDisplay = XOpenDisplay(NULL);

    if (fbDisplay == 0) {
        HIPHOP_LOG_STDERR("Cannot open display");
        return;
    }

    int fd = ipc_get_config(fIpc)->fd_r;
    fd_set rfds;
    struct timeval tv;
    tlv_t packet;

    fbRunMainLoop = true;
    
    while (fbRunMainLoop) {
        CefDoMessageLoopWork();

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = tv.tv_usec = 0; // poll

        int retval = select(fd + 1, &rfds, 0, 0, &tv);

        if (retval == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Failed select() on IPC channel");
            fbRunMainLoop = false;
            continue;
        }

        if (retval == 0) {
            continue; // no fd ready
        }

        if (ipc_read(fIpc, &packet) == -1) {
            HIPHOP_LOG_STDERR_ERRNO("Could not read from IPC channel");
            fbRunMainLoop = false;
            continue;
        }

        dispatch(&packet);
    }
}

// Called in browser and renderer processes
bool CefHelper::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefProcessId sourceProcess,
                                         CefRefPtr<CefProcessMessage> message)
{
    if ((sourceProcess == PID_BROWSER) && (message->GetName() == "inject_script")) {
        frFullInjectedScript = message->GetArgumentList()->GetString(0);
        return true;
    }

    return false;
}

// Called in browser process
void CefHelper::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine)
{
    // Renderer process owns the JavaScript callback and needs writing back to host
    const ipc_conf_t* conf = ipc_get_config(fIpc);
    commandLine->AppendSwitchWithValue("fdw", std::to_string(conf->fd_w));

    // Set some Chromium options
    commandLine->AppendSwitch("disable-extensions");
}

// Called in renderer process
void CefHelper::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context)
{
    // V8 context is ready, first define the window.hostPostMessage function.
    CefRefPtr<CefV8Value> window = context->GetGlobal();
    window->SetValue("hostPostMessage", CefV8Value::CreateFunction("hostPostMessage", this),
                     V8_PROPERTY_ATTRIBUTE_NONE);

    // Then run queued injected script
    frame->ExecuteJavaScript(frFullInjectedScript, frame->GetURL(), 0);
}

// Called in browser and renderer processes
void CefHelper::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          int httpStatusCode)
{
    if (!fbRunMainLoop) {
        return;
    }

    XMapWindow(fbDisplay, fbContainer);
    XSync(fbDisplay, False);

    // TODO
}

// Called in renderer process
bool CefHelper::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                        CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    if ((name != "hostPostMessage") || (arguments.size() != 1) || (!arguments[0]->IsArray())) {
        HIPHOP_LOG_STDERR_COLOR("Invalid call to host");
        return false;
    }

    CefRefPtr<CefV8Value> args = arguments[0];

    printf("FIXME : hostPostMessage() called with %d arguments\n", args->GetArrayLength());

    return true;
}

void CefHelper::dispatch(const tlv_t* packet)
{
    switch (static_cast<msg_opcode_t>(packet->t)) {
        case OP_REALIZE:
            realize((const msg_win_cfg_t *)packet->v);
            break;

        case OP_NAVIGATE: {
            const char* url = static_cast<const char*>(packet->v);
            fbBrowser->GetMainFrame()->LoadURL(url);
            break;
        }

        case OP_RUN_SCRIPT:
            fbBrowser->GetMainFrame()->ExecuteJavaScript(static_cast<const char*>(packet->v), "", 0);
            break;

        case OP_INJECT_SCRIPT: {
            const char *js = static_cast<const char*>(packet->v);
            fbInjectedScripts.push_back(js);
            break;
        }

        case OP_SET_SIZE: {
            // TODO - untested
            /*const msg_win_size_t *size = (const msg_win_size_t *)packet->v;
            ::Display* display = cef_get_xdisplay();
            ::Window window = static_cast<::Window>(fbBrowser->GetHost()->GetWindowHandle());
            XResizeWindow(display, window, size->width, size->height);
            XSync(display, False);*/
            break;
        }

        case OP_SET_KEYBOARD_FOCUS:
            // TODO
            break;

        case OP_TERMINATE:
            fbRunMainLoop = false;
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
    XMatchVisualInfo(fbDisplay, DefaultScreen(fbDisplay), 24, TrueColor, &vinfo);

    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(fbDisplay, XDefaultRootWindow(fbDisplay),
                                     vinfo.visual, AllocNone);

    fbContainer = XCreateWindow(fbDisplay, static_cast<::Window>(config->parent),
                               0, 0, config->size.width, config->size.height, 0,
                               vinfo.depth, CopyFromParent, vinfo.visual,
                               CWColormap, &attrs);
    XSync(fbDisplay, False);

    CefBrowserSettings settings;

    // TODO
    //settings.log_severity = DISABLE;

    CefWindowInfo windowInfo;
    windowInfo.parent_window = fbContainer;
    windowInfo.width = config->size.width;
    windowInfo.height = config->size.height;

    fbBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings,
        nullptr, nullptr);

    // Injecting a script means queuing it to run right before document starts
    // loading to ensure they run before any user script. The V8 context must
    // be already initialized in order to run scripts. Send the script to
    // renderer because V8 ready event (OnContextCreated) only fires in there.
    
    std::string js = "";

    for (std::vector<CefString>::iterator it = fbInjectedScripts.begin(); it != fbInjectedScripts.end(); ++it) {
        js += *it;
    }

    js += JS_POST_MESSAGE_SHIM;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("inject_script");
    message->GetArgumentList()->SetString(0, js);
    fbBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
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
