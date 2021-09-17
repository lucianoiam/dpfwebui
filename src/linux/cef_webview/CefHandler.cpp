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

#include "CefHandler.h"

#include <sstream>
#include <string>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

CefHandler* sInstance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mimeType)
{
    return "data:" + mimeType + ";base64," +
            CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
            .ToString();
}

CefHandler::CefHandler()
{
    DCHECK(!sInstance);
    sInstance = this;
}

CefHandler::~CefHandler()
{
    sInstance = nullptr;
}

// static
CefHandler* CefHandler::GetInstance()
{
    return sInstance;
}

void CefHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();

    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
}

bool CefHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               const CefString& target_url, 
                               const CefString& target_frame_name,
                               CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                               bool user_gesture,
                               const CefPopupFeatures& popupFeatures,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings,
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool* no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();

    // Disable popups, only the main browser is allowed
    return true;
}

void CefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Keep browser.
    fBrowser = browser;
}

bool CefHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void CefHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();

    fBrowser = nullptr;
}

void CefHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
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

void CefHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    std::string titleStr(title);

    // Retrieve the X11 display shared with Chromium.
    ::Display* display = cef_get_xdisplay();
    DCHECK(display);

    // Retrieve the X11 window handle for the browser.
    ::Window window = browser->GetHost()->GetWindowHandle();
    if (window == kNullWindowHandle) {
        return;
    }

    // Retrieve the atoms required by the below XChangeProperty call.
    const char* kAtoms[] = {"_NET_WM_NAME", "UTF8_STRING"};
    Atom atoms[2];
    int result = XInternAtoms(display, const_cast<char**>(kAtoms), 2, false, atoms);
    if (!result) {
        NOTREACHED();
    }

    // Set the window title.
    XChangeProperty(display, window, atoms[0], atoms[1], 8, PropModeReplace,
        reinterpret_cast<const unsigned char*>(titleStr.c_str()), titleStr.size());

    // TODO(erg): This is technically wrong. So XStoreName and friends expect
    // this in Host Portable Character Encoding instead of UTF-8, which I believe
    // is Compound Text. This shouldn't matter 90% of the time since this is the
    // fallback to the UTF8 property above.
    XStoreName(display, browser->GetHost()->GetWindowHandle(), titleStr.c_str());
}
