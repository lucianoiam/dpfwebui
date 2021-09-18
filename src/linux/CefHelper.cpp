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
#include <string>
#include <sys/select.h>

#if defined(CEF_X11)
#include <X11/Xlib.h>

static int XErrorHandlerImpl(Display* display, XErrorEvent* event);
static int XIOErrorHandlerImpl(Display* display);
#endif

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "macro.h"
#include "ipc_message.h"

// Entry point function for all processes
int main(int argc, char* argv[])
{
    // Provide CEF with command-line arguments
    CefMainArgs main_args(argc, argv);

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int code = CefExecuteProcess(main_args, nullptr, nullptr);
    if (code >= 0) {
        // The sub-process has completed so return here
        return code;
    }

    // Initialize custom (non-CEF) IPC channel
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

#if defined(CEF_X11)
    // Install xlib error handlers so that the application won't be terminated
    // on non-fatal errors
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

    // Parse command-line arguments for use in this method
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv);

    // Specify CEF global settings here
    CefSettings settings;
    //settings.no_sandbox = true;

    // CefHelper implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<CefHelper> app(new CefHelper(ipc));

    // Initialize CEF for the browser process.
    CefInitialize(main_args, settings, app.get(), nullptr);

    // Run main loop
    app.get()->run();

    // Cleanup
    CefShutdown();
    ipc_destroy(ipc);

    return 0;
}

CefHelper::CefHelper(ipc_t* ipc) : fRun(false), fIpc(ipc)
{}

void CefHelper::run()
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

CefRefPtr<CefBrowserProcessHandler> CefHelper::GetBrowserProcessHandler()
{
    return this;
}

void CefHelper::dispatch(const tlv_t* packet)
{
    switch (static_cast<msg_opcode_t>(packet->t)) {
        case OP_TERMINATE:
            fRun = false;
            break;

        default:
            break;
    }
}

void CefHelper::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    // CefHelperHandler implements browser-level callbacks.
    CefRefPtr<CefHelperHandler> handler(new CefHelperHandler());

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    std::string url = "http://duckduckgo.com";

    // Information used when creating the native window.
    CefWindowInfo window_info;

    // Create the browser window.
    fBrowser = CefBrowserHost::CreateBrowserSync(window_info, handler, url,
        browser_settings, nullptr, nullptr);
}

CefHelperHandler::CefHelperHandler()
{}

CefHelperHandler::~CefHelperHandler()
{}

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mimeType)
{
    return "data:" + mimeType + ";base64," +
            CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
            .ToString();
}

void CefHelperHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED) {
        return;
    }

    // Display a load error message using a data: URI.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
          "<h2>Failed to load URL "
       << std::string(failedUrl) << " with error " << std::string(errorText)
       << " (" << errorCode << ").</h2></body></html>";

    frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

#if defined(CEF_X11)
static int XErrorHandlerImpl(Display* display, XErrorEvent* event)
{
    LOG(WARNING) << "X error received: "
                 << "type " << event->type << ", "
                 << "serial " << event->serial << ", "
                 << "error_code " << static_cast<int>(event->error_code) << ", "
                 << "request_code " << static_cast<int>(event->request_code)
                 << ", "
                 << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

static int XIOErrorHandlerImpl(Display* display)
{
    return 0;
}
#endif  // defined(CEF_X11)
