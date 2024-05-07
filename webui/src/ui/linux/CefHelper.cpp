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

#include "CefHelper.hpp"

#include <cstddef>
#include <sstream>

#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>

#include "distrho/extra/sofd/libsofd.h"
#include "extra/Path.hpp"
#include "scaling.h"

#define JS_POST_MESSAGE_SHIM "window.host.postMessage = (payload) => window.hostPostMessage(payload);"

static int XErrorHandlerImpl(Display* display, XErrorEvent* event)
{
    std::stringstream ss;

    ss << "X error received: "
       << "type " << event->type << ", "
       << "serial " << event->serial << ", "
       << "error_code " << static_cast<int>(event->error_code) << ", "
       << "request_code " << static_cast<int>(event->request_code) << ", "
       << "minor_code " << static_cast<int>(event->minor_code);

    d_stderr2(ss.str().c_str());
    
    return 0;
}

static int XIOErrorHandlerImpl(Display* display)
{
    return 0;
}

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
    : fRunMainLoop(false)
    , fIpc(nullptr)
    , fDisplay(nullptr)
    , fContainer(0)
    , fBrowser(nullptr)
    , fScripts(nullptr)
    , fDialogCallback(nullptr)
{
    fScripts = CefListValue::Create();
}

CefHelper::~CefHelper()
{
    if (fIpc != nullptr) {
        delete fIpc;
    }

    if (fContainer != 0) {
        XDestroyWindow(fDisplay, fContainer);
    }

    if (fDisplay != nullptr) {
        XCloseDisplay(fDisplay);
    }
}

int CefHelper::run(const CefMainArgs& args)
{
    // Parse command line arguments and create IPC channel
    if (args.argc < 3) {
        d_stderr("Invalid argument count");
        return -1;
    }

    const int fdr = std::atoi(args.argv[1]);
    const int fdw = std::atoi(args.argv[2]);

    if ((fdr == 0) || (fdw == 0)) {
        d_stderr("Invalid file descriptor");
        return -1;
    }

    fIpc = new IpcChannel(fdr, fdw, 50/*read timeout ms*/);

    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    fDisplay = XOpenDisplay(NULL);

    if (fDisplay == 0) {
        d_stderr("Cannot open display");
        return -1;
    }

    CefSettings settings;
    settings.log_severity = LOGSEVERITY_DISABLE;
    settings.chrome_runtime = false;
    CefString(&settings.cache_path) = Path::getUserData();
    //CefString(&settings.user_agent) = ... does not work for WebSockets

    // Initialize CEF for the browser process
    CefInitialize(args, settings, this, nullptr);

    // Let parent process know child is ready
    const float dpr = device_pixel_ratio();
    fIpc->write(OP_HANDLE_INIT, &dpr, sizeof(dpr));

    runMainLoop();

    // fBrowser must be deleted before calling CefShutdown() otherwise program
    // can hang or crash
    fBrowser = nullptr;

    CefShutdown();

    return 0;
}

void CefHelper::runMainLoop()
{
    XEvent event;
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

        // Read the plugin<->helper IPC pipe
        // Use a non-zero timeout for throotling down CefDoMessageLoopWork()
        rc = fIpc->read(&packet);

        if (rc == 0) {
            dispatch(packet);
        }

        // Handle libSOFD file dialog
        if (XPending(fDisplay) > 0) {
            XNextEvent(fDisplay, &event);

            rc = x_fib_handle_events(fDisplay, &event);

            if (rc > 0) {
                fDialogCallback->Continue({ x_fib_filename() });
                fDialogCallback = nullptr;
            } else if (rc < 0) {
                fDialogCallback->Cancel();
                fDialogCallback = nullptr;
            }
        }
    }
}

void CefHelper::dispatch(const tlv_t& packet)
{
    switch (static_cast<msg_opcode_t>(packet.t)) {
        case OP_REALIZE:
            realize(static_cast<const msg_view_cfg_t*>(packet.v));
            break;
        case OP_NAVIGATE:
            navigate(static_cast<const char*>(packet.v));
            break;
        case OP_RUN_SCRIPT:
            runScript(static_cast<const char*>(packet.v));
            break;
        case OP_INJECT_SHIMS:
            injectScript(JS_POST_MESSAGE_SHIM);
            break;
        case OP_INJECT_SCRIPT: 
            injectScript(static_cast<const char*>(packet.v));
            break;
        case OP_SET_SIZE:
            setSize(static_cast<const msg_view_size_t*>(packet.v));
            break;
        case OP_SET_KEYBOARD_FOCUS:
            setKeyboardFocus(*static_cast<const bool*>(packet.v) == 1);
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
    if (sourceProcess != PID_RENDERER) {
        return false;
    }

    if (message->GetName() == "HostPostMessage") {
        const CefRefPtr<CefBinaryValue> val = message->GetArgumentList()->GetBinary(0);
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
    // This was needed for older versions of CEF
    // Chromium weird scaling https://magpcss.org/ceforum/viewtopic.php?t=11491
    //const float zoomLevel = std::log(device_pixel_ratio()) / std::log(1.2f);
    //browser->GetHost()->SetZoomLevel(zoomLevel);
}

void CefHelper::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          int httpStatusCode)
{
    XMapWindow(fDisplay, fContainer);
    fIpc->write(OP_HANDLE_LOAD_FINISHED);
    // TODO : look for a better solution than a delay to prevent white flicker
    usleep(50000L);
}

CefRefPtr<CefResourceRequestHandler> 
CefHelper::GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool is_navigation,
                                     bool is_download,
                                     const CefString& request_initiator,
                                     bool& disable_default_handling)
{
    if (is_navigation && (fScripts->GetSize() > 0)) {
        fIndexHtml.clear();
        return this;
    }

    return nullptr;
}

CefResourceRequestHandler::ReturnValue
CefHelper::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefCallback> callback)
{
    // TODO : this only seems to affect the main document request headers,
    //        not resource requests or WebSockets.
    
    /*CefRequest::HeaderMap headers;
    request->GetHeaderMap(headers);
    CefRequest::HeaderMap::iterator it = headers.find("User-Agent");

    if (it != headers.end()) {
        std::string userAgent = it->second.ToString() + " " + fUserAgent;
        headers.erase(it);
        headers.insert(CefRequest::HeaderMap::value_type("User-Agent", userAgent));
        request->SetHeaderMap(headers);
    }*/

    return RV_CONTINUE;
}

CefResponseFilter::FilterStatus
CefHelper::Filter(void* data_in,
                  size_t data_in_size,
                  size_t& data_in_read,
                  void* data_out,
                  size_t data_out_size,
                  size_t& data_out_written)
{
    data_in_read = data_in_size;
    data_out_written = 0;

    if (data_in_size == 0) {
        d_stderr2("Could not find injection point for scripts");
        return RESPONSE_FILTER_ERROR;
    }

    // Inject scripts before first script in document or before end of body

    fIndexHtml += std::string(static_cast<char*>(data_in), data_in_size);
    size_t pos = fIndexHtml.find("<script");

    if (pos == std::string::npos) {
        pos = fIndexHtml.find("</body");

        if (pos == std::string::npos) {
            return RESPONSE_FILTER_NEED_MORE_DATA;
        }
    }

    std::string scripts = "<script>";
    const size_t scriptCount = fScripts->GetSize();

    for (size_t i = 0; i < scriptCount; ++i) {
        scripts += fScripts->GetString(i).ToString() + ";";
    }

    scripts += "</script>";
    
    fIndexHtml.insert(pos, scripts);
    const size_t indexSize = fIndexHtml.size();

    if (data_out_size < indexSize) {
        d_stderr2("Output buffer too small %lu < %lu", data_out_size, indexSize);
        return RESPONSE_FILTER_ERROR;
    }

    fIndexHtml.copy(static_cast<char*>(data_out), indexSize);
    fIndexHtml.clear();
    
    data_out_written = indexSize;

    return RESPONSE_FILTER_DONE;
}

bool CefHelper::OnFileDialog(CefRefPtr<CefBrowser> browser,                    
                             FileDialogMode mode,                              
                             const CefString& title,                           
                             const CefString& defaultFilePath,                 
                             const std::vector<CefString>& acceptFilters,
                             CefRefPtr<CefFileDialogCallback> callback)
{
    if (fDialogCallback != nullptr) {
        callback->Cancel(); // only a single dialog is supported
        return true;
    }

    if (x_fib_configure(1 /*current dir*/, defaultFilePath.ToString().c_str()) != 0) {
        callback->Cancel();
        return true;
    }

    if (x_fib_configure(1 /*set title*/, title.ToString().c_str()) != 0) {
        callback->Cancel();
        return true;
    }

    if (x_fib_cfg_buttons(2 /*show places*/, 1 /*checked*/) != 0) {
        callback->Cancel();
        return true;
    }

    // Web view scaling value is too small for libSOFD, bump it up.
    const double scaleFactor = std::ceil(static_cast<double>(device_pixel_ratio()));
    if (x_fib_show(fDisplay, 0 /*parent*/, 0, 0, scaleFactor) != 0) {
        callback->Cancel();
        return true;
    }

    fDialogCallback = callback;

    return true;
}

void CefHelper::realize(const msg_view_cfg_t* config)
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
    XFlush(fDisplay);

    CefWindowInfo windowInfo;
    windowInfo.SetAsChild(fContainer, {});

    const CefBrowserSettings settings;

    fBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings,
        nullptr, nullptr);

    const ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
    XSetWindowBackground(fDisplay, w, config->color); // reduce artifacts when resizing
    XResizeWindow(fDisplay, w, config->size.width, config->size.height);

    fUserAgent = config->userAgent;
}

void CefHelper::navigate(const char* url)
{
    fBrowser->GetMainFrame()->LoadURL(url);
}

void CefHelper::runScript(const char* js)
{
    const CefRefPtr<CefFrame> frame = fBrowser->GetMainFrame();
    frame->ExecuteJavaScript(js, frame->GetURL(), 0);
}

void CefHelper::injectScript(const char* js)
{
    fScripts->SetString(fScripts->GetSize(), js);
}

void CefHelper::setSize(const msg_view_size_t* size)
{
    const unsigned width = size->width;
    const unsigned height = size->height;

    if (fContainer != 0) {
        XResizeWindow(fDisplay, fContainer, width, height);
    }

    if (fBrowser != nullptr) {
        ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
        XResizeWindow(fDisplay, w, width, height);
    }
}

void CefHelper::setKeyboardFocus(bool keyboardFocus)
{
    if (fBrowser == nullptr) {
        return;
    }
    
    if (keyboardFocus) {
        // Toggling keyboard focus works but generates Xlib errors when
        // keyboardFocus==true. Possibly incorrect arguments for XIGrabDevice().
        // type 0, error_code 129, request_code 131, minor_code 51 (and 52)
        const ::Window w = static_cast<::Window>(fBrowser->GetHost()->GetWindowHandle());
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

void CefSubprocess::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context)
{
    // V8 context is ready, define the window.hostPostMessage function.
    CefRefPtr<CefV8Value> window = context->GetGlobal();
    window->SetValue("hostPostMessage", CefV8Value::CreateFunction("hostPostMessage", this),
                     V8_PROPERTY_ATTRIBUTE_NONE);
    fBrowser = browser;
}

bool CefSubprocess::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                                  CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    if ((name != "hostPostMessage") || (arguments.size() != 1) || (!arguments[0]->IsArray())) {
        d_stderr2("Invalid call to host");
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

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("HostPostMessage");
    message->GetArgumentList()->SetBinary(0, CefBinaryValue::Create(payload, offset));
    fBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);

    if (payload) {
        free(payload);
    }

    return true;
}
