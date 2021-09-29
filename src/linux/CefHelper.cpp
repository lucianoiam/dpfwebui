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

#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>

#include "Path.hpp"
#include "macro.h"

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.hostPostMessage(args);"

static int XErrorHandlerImpl(Display* display, XErrorEvent* event);
static int XIOErrorHandlerImpl(Display* display);

// Entry point function for all processes
int main(int argc, char* argv[])
{
    CefMainArgs args(argc, argv);
    CefRefPtr<CefSubprocess> proc = new CefSubprocess();

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int code = CefExecuteProcess(args, proc.get(), nullptr);
    if (code >= 0) {
        // The sub-process has completed so return here
        return code;
    }

    // Run main CEF browser process
    CefHelper helper;
    code = helper.run(args);

    return code;
}

CefHelper::CefHelper()
    : fIpc(0)
    , fRunMainLoop(false)
    , fDisplay(0)
    , fContainer(0)
{
    fInjectedScripts = CefListValue::Create();
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

int CefHelper::run(const CefMainArgs& args)
{
    // Parse command line arguments and create IPC channel
    if (args.argc < 3) {
        HIPHOP_LOG_STDERR("Invalid argument count");
        return -1;
    }

    int fdr = std::atoi(args.argv[1]);
    int fdw = std::atoi(args.argv[2]);

    if ((fdr == 0) || (fdw == 0)) {
        HIPHOP_LOG_STDERR("Invalid file descriptor");
        return -1;
    }

    fIpc = new IpcChannel(fdr, fdw);

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
    settings.log_severity = LOGSEVERITY_DISABLE;
    settings.chrome_runtime = false;
    CefString(&settings.cache_path) = path::getCachesPath();

    // Initialize CEF for the browser process
    CefInitialize(args, settings, this, nullptr);

    runMainLoop();

    // fBrowser must be deleted before calling CefShutdown() otherwise program
    // can hang or crash
    fBrowser = nullptr;

    CefShutdown();

    return 0;
}

void CefHelper::runMainLoop()
{
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

        // Use a non-zero timeout for throotling down CefDoMessageLoopWork()
        rc = fIpc->read(&packet, 5/* ms */);

        if (rc == 0) {
            dispatch(packet);
        }
    }
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
            const char* js = static_cast<const char*>(packet.v);
            fInjectedScripts->SetString(fInjectedScripts->GetSize(), js);
            break;
        }

        case OP_SET_SIZE: {
            const msg_win_size_t* size = static_cast<const msg_win_size_t*>(packet.v);
            ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
            XResizeWindow(fDisplay, w, size->width, size->height);
            XResizeWindow(fDisplay, fContainer, size->width, size->height);
            XSync(fDisplay, False);
            break;
        }

        case OP_SET_KEYBOARD_FOCUS:
            setKeyboardFocus(*((char *)packet.v) == 1);
            break;

        case OP_TERMINATE:
            fRunMainLoop = false;
            break;

        default:
            break;
    }
}

bool CefHelper::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefProcessId sourceProcess,
                                         CefRefPtr<CefProcessMessage> message)
{
    if ((sourceProcess == PID_RENDERER) && (message->GetName() == "ScriptMessage")) {
        CefRefPtr<CefBinaryValue> val = message->GetArgumentList()->GetBinary(0);
        char payload[val->GetSize()];
        val->GetData(payload, sizeof(payload), 0);
        fIpc->write(OP_HANDLE_SCRIPT_MESSAGE, payload, sizeof(payload));
        return true;
    }

    return false;
}

void CefHelper::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine)
{
    // https://peter.sh/experiments/chromium-command-line-switches/
    commandLine->AppendSwitch("disable-extensions");
}

void CefHelper::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                            TransitionType transitionType)
{
    browser->GetHost()->SetZoomLevel(getZoomLevel());
}

void CefHelper::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          int httpStatusCode)
{
    XMapWindow(fDisplay, fContainer);
    XSync(fDisplay, False);

    fIpc->write(OP_HANDLE_LOAD_FINISHED);
}

void CefHelper::realize(const msg_win_cfg_t* config)
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

    CefWindowInfo windowInfo;
    windowInfo.parent_window = fContainer;
    windowInfo.width = config->size.width;
    windowInfo.height = config->size.height;

    CefBrowserSettings settings;

    fBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings,
        nullptr, nullptr);

    // Injecting a script means queuing it to run right before document starts
    // loading to ensure they run before any user script. The V8 context must
    // be already initialized in order to run scripts. V8 init callback fires in
    // the renderer process. Send the scripts to the renderer process to have
    // them called from there. Sending a context init event from the renderer
    // to the browser process to run scripts from the browser process does not
    // guarantee injected scripts will run before user scripts due to timing.

    fInjectedScripts->SetString(fInjectedScripts->GetSize(), JS_POST_MESSAGE_SHIM);
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("InjectScripts");
    message->GetArgumentList()->SetList(0, fInjectedScripts);
    fBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);

    // Reduce artifacts when resizing
    ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
    XSetWindowBackground(fDisplay, w, config->color);
}

// Match value returned by LinuxWebHostUI::getDisplayScaleFactor()
float CefHelper::getZoomLevel()
{
    XrmInitialize();

    if (char* const rms = XResourceManagerString(fDisplay)) {
        if (const XrmDatabase sdb = XrmGetStringDatabase(rms)) {
            char* type = nullptr;
            XrmValue ret;

            if (XrmGetResource(sdb, "Xft.dpi", "String", &type, &ret)
                    && (ret.addr != nullptr) && (type != nullptr)
                    && (std::strncmp("String", type, 6) == 0)) {
                if (const float dpi = std::atof(ret.addr)) {
                    return dpi / 96;
                }
            }
        }
    }

    return 1.f;
}

void CefHelper::setKeyboardFocus(bool keyboardFocus)
{
    if (keyboardFocus) {
        // CEFKBDFOCUSBUG - This works but generates Xlib errors
        // type 0, error_code 129, request_code 131, minor_code 51 (and 52)
        ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
        XSetInputFocus(fDisplay, w, RevertToNone, CurrentTime);
        XIEventMask evmask;
        evmask.deviceid = XIAllMasterDevices;
        evmask.mask_len = XIMaskLen(XI_LASTEVENT);
        unsigned char mask [evmask.mask_len];
        memset(mask, 0, sizeof(mask));
        XISetMask(mask, XI_RawKeyPress);
        XISetMask(mask, XI_RawKeyRelease);
        evmask.mask = mask;
        XIGrabDevice(fDisplay, XIAllMasterDevices, fContainer, CurrentTime, None,
            GrabModeAsync, GrabModeAsync, False, &evmask);
    } else {
        XIUngrabDevice(fDisplay, XIAllMasterDevices, CurrentTime);
    }
}

CefSubprocess::CefSubprocess()
{
    fInjectedScripts = CefListValue::Create();
}

bool CefSubprocess::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefFrame> frame,
                                                   CefProcessId sourceProcess,
                                                   CefRefPtr<CefProcessMessage> message)
{
    if ((sourceProcess == PID_BROWSER) && (message->GetName() == "InjectScripts")) {
        fInjectedScripts = message->GetArgumentList()->GetList(0)->Copy();
        return true;
    }

    return false;
}

void CefSubprocess::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context)
{
    fBrowser = browser;

    // V8 context is ready, first define the window.hostPostMessage function.
    CefRefPtr<CefV8Value> window = context->GetGlobal();
    window->SetValue("hostPostMessage", CefV8Value::CreateFunction("hostPostMessage", this),
                     V8_PROPERTY_ATTRIBUTE_NONE);

    // Then run queued injected scripts
    for (int i = 0; i < fInjectedScripts->GetSize(); ++i) {
        frame->ExecuteJavaScript(fInjectedScripts->GetString(i), frame->GetURL(), 0);
    }
}

bool CefSubprocess::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                                  CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    if ((name != "hostPostMessage") || (arguments.size() != 1) || (!arguments[0]->IsArray())) {
        HIPHOP_LOG_STDERR_COLOR("Invalid call to host");
        return false;
    }

    char *payload = NULL;
    int offset = 0;

    for (int i = 0; i < arguments[0]->GetArrayLength(); i++) {
        CefRefPtr<CefV8Value> arg = arguments[0]->GetValue(i);

        if (arg->IsBool()) {
            payload = static_cast<char *>(realloc(payload, offset + 1));
            int b = arg->GetBoolValue() ? ARG_TYPE_TRUE : ARG_TYPE_FALSE;
            *(payload+offset) = static_cast<char>(b);
            offset += 1;

        } else if (arg->IsDouble()) {
            payload = static_cast<char *>(realloc(payload, offset + 1 + sizeof(double)));
            *(payload+offset) = static_cast<char>(ARG_TYPE_DOUBLE);
            offset += 1;
            *reinterpret_cast<double *>(payload+offset) = arg->GetDoubleValue();
            offset += sizeof(double);

        } else if (arg->IsString()) {
            std::string s = arg->GetStringValue().ToString();
            int slen = s.length() + 1;
            payload = static_cast<char *>(realloc(payload, offset + 1 + slen));
            *(payload+offset) = static_cast<char>(ARG_TYPE_STRING);
            offset += 1;
            strcpy(payload+offset, s.c_str());
            offset += slen;

        } else {
            payload = static_cast<char *>(realloc(payload, offset + 1));
            *(payload+offset) = static_cast<char>(ARG_TYPE_NULL);
            offset += 1;
        }
    }

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("ScriptMessage");
    message->GetArgumentList()->SetBinary(0, CefBinaryValue::Create(payload, offset));
    fBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);

    if (payload) {
        free(payload);
    }

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
