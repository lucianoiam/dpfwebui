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

#ifndef CEF_HELPER_HPP
#define CEF_HELPER_HPP

#include "include/cef_app.h"
#include "include/cef_client.h"

#include "ipc.h"

// Implement application-level callbacks for the browser process.
class CefHelper : public CefApp, public CefBrowserProcessHandler
{
public:
    CefHelper(ipc_t* ipc);

    void run();

    // CefApp methods

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

    // CefBrowserProcessHandler methods

    virtual void OnContextInitialized() override;

private:
    void dispatch(const tlv_t* packet);

    bool   fRun;
    ipc_t* fIpc;

    CefRefPtr<CefBrowser> fBrowser;

    // Include the default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelper);
};


// TODO : consider merging CefHelperHandler into CefHelper, first determine
//        which handler methods are needed

class CefHelperHandler : public CefClient, public CefLoadHandler
{
public:
    explicit CefHelperHandler();
    ~CefHelperHandler();

    // CefClient methods

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }

    // CefLoadHandler methods

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) override;

private:
    // Include the default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelperHandler);
};

#endif  // CEF_HELPER_HPP
