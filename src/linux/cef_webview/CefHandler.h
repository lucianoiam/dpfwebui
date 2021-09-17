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

#ifndef CEF_HANDLER_HPP
#define CEF_HANDLER_HPP

#include <list>

#include "include/cef_client.h"

class CefHandler : public CefClient, public CefDisplayHandler,
                   public CefLifeSpanHandler, public CefLoadHandler
{
public:
    explicit CefHandler();
    ~CefHandler();

    // Provide access to the single global instance of this object
    static CefHandler* GetInstance();

    // CefClient methods

    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }

    // CefDisplayHandler methods

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

    // CefLifeSpanHandler methods

    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               const CefString& target_url, 
                               const CefString& target_frame_name,
                               CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                               bool user_gesture,
                               const CefPopupFeatures& popupFeatures,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings,
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool* no_javascript_access) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) override;

private:
    // Platform-specific implementation
    void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);

    // Existing browser windows. Only accessed on the CEF UI thread.
    CefRefPtr<CefBrowser> fBrowser;

    // Include the default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHandler);
};

#endif  // CEF_HANDLER_HPP
