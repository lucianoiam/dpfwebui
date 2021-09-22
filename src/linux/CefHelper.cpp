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

#include <X11/Xutil.h>

#include "macro.h"

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.hostPostMessage(args);"

static int XErrorHandlerImpl(Display* display, XErrorEvent* event);
static int XIOErrorHandlerImpl(Display* display);

// Entry point function for all processes
int main(int argc, char* argv[])
{
    CefMainArgs args(argc, argv);
    CefRefPtr<CefHelperSubprocess> proc = new CefHelperSubprocess();

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int code = CefExecuteProcess(args, proc.get(), nullptr);
    if (code >= 0) {
        // The sub-process has completed so return here
        return code;
    }

    int fdr, fdw;

    if (argc < 3) {
        HIPHOP_LOG_STDERR("Invalid argument count");
        return -1;
    }

    if ((sscanf(argv[1], "%d", &fdr) == 0) || (sscanf(argv[2], "%d", &fdw) == 0)) {
        HIPHOP_LOG_STDERR("Invalid file descriptor");
        return -1;
    }

    CefHelper helper(fdr, fdw);
    code = helper.run();

    return code;
}

CefHelper::CefHelper(int fdr, int fdw)
    : fIpc(0)
    , fRunMainLoop(false)
    , fDisplay(0)
    , fContainer(0)
{
    fIpc = new IpcInterface(fdr, fdw, 5);
}

CefHelper::~CefHelper()
{
    if (fIpc != 0) {
        delete fIpc;
    }

    if (fContainer != 0) {
        XDestroyWindow(fDisplay, fContainer);
    }

    if (fDisplay != 0) {
        XCloseDisplay(fDisplay);
    }
}

int CefHelper::run()
{
    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    fDisplay = XOpenDisplay(NULL);

    if (fDisplay == 0) {
        HIPHOP_LOG_STDERR("Cannot open display");
        return -1;
    }

    CefSettings settings;
    //settings.no_sandbox = true;
    settings.chrome_runtime = false;

    // Initialize CEF for the browser process
    CefMainArgs emptyArgs(0, nullptr);
    CefInitialize(emptyArgs, settings, this, nullptr);

    tlv_t packet;
    int rc;

    fRunMainLoop = true;
    
    while (fRunMainLoop) {
        // Call CefDoMessageLoopWork() on a regular basis instead of calling
        // CefRunMessageLoop(). Each call to CefDoMessageLoopWork() will perform
        // a single iteration of the CEF message loop. Caution should be used
        // with this approach. Calling the method too infrequently will starve
        // the CEF message loop and negatively impact browser performance.
        // Calling the method too frequently will negatively impact CPU usage.
        // https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage
        CefDoMessageLoopWork();

        // Configured to timeout after 5ms when no data is available
        rc = fIpc->read(&packet);

        if (rc == 0) {
            dispatch(packet);
        }
    }

    // fBrowser must be deleted before calling CefShutdown() otherwise program
    // can hang or crash
    fBrowser = nullptr;

    CefShutdown();

    return 0;
}

void CefHelper::dispatch(const tlv_t& packet)
{
    switch (static_cast<msg_opcode_t>(packet.t)) {
        case OP_REALIZE:
            realize((const msg_win_cfg_t *)packet.v);
            break;

        case OP_NAVIGATE: {
            const char* url = static_cast<const char*>(packet.v);
            fBrowser->GetMainFrame()->LoadURL(url);
            break;
        }

        case OP_RUN_SCRIPT: {
            const char* js = static_cast<const char*>(packet.v);
            CefRefPtr<CefFrame> frame = fBrowser->GetMainFrame();
            frame->ExecuteJavaScript(js, frame->GetURL(), 0);
            break;
        }

        case OP_INJECT_SCRIPT: {
            fInjectedScript += static_cast<const char*>(packet.v);
            break;
        }

        case OP_SET_SIZE: {
            // TODO - untested
            /*const msg_win_size_t *size = (const msg_win_size_t *)packet.v;
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
            fRunMainLoop = false;
            break;

        default:
            break;
    }
}

void CefHelper::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine)
{
    // Renderer process owns the JavaScript callback and needs writing back to host
    commandLine->AppendSwitchWithValue("ipc-fd", std::to_string(fIpc->getFdRead()));

    // Set some Chromium options
    commandLine->AppendSwitch("disable-extensions");
}

void CefHelper::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          int httpStatusCode)
{
    if (!fRunMainLoop) {
        return;
    }

    XMapWindow(fDisplay, fContainer);
    XSync(fDisplay, False);

    // TODO
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
    XSync(fDisplay, False);

    CefBrowserSettings settings;

    // TODO
    //settings.log_severity = DISABLE;

    CefWindowInfo windowInfo;
    windowInfo.parent_window = fContainer;
    windowInfo.width = config->size.width;
    windowInfo.height = config->size.height;

    fBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings,
        nullptr, nullptr);

    // Injecting a script means queuing it to run right before document starts
    // loading to ensure they run before any user script. The V8 context must
    // be already initialized in order to run scripts. Send the script to
    // renderer because V8 ready event (OnContextCreated) only fires in there.
    
    fInjectedScript += JS_POST_MESSAGE_SHIM;
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("inject_script");
    message->GetArgumentList()->SetString(0, fInjectedScript);
    fBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
}

CefHelperSubprocess::CefHelperSubprocess()
    : fIpc(0)
{
    // TODO - create IpcInterface using CEF arguments
}

CefHelperSubprocess::~CefHelperSubprocess()
{
    if (fIpc != 0) {
        delete fIpc;
    }
}

bool CefHelperSubprocess::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefFrame> frame,
                                                   CefProcessId sourceProcess,
                                                   CefRefPtr<CefProcessMessage> message)
{
    if ((sourceProcess == PID_BROWSER) && (message->GetName() == "inject_script")) {
        fInjectedScript = message->GetArgumentList()->GetString(0);
        return true;
    }

    return false;
}

void CefHelperSubprocess::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context)
{
    // V8 context is ready, first define the window.hostPostMessage function.
    CefRefPtr<CefV8Value> window = context->GetGlobal();
    window->SetValue("hostPostMessage", CefV8Value::CreateFunction("hostPostMessage", this),
                     V8_PROPERTY_ATTRIBUTE_NONE);

    // Then run queued injected script
    frame->ExecuteJavaScript(fInjectedScript, frame->GetURL(), 0);
}

bool CefHelperSubprocess::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
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
