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

#ifndef EDGEWEBWIDGET_HPP
#define EDGEWEBWIDGET_HPP

#define UNICODE
#define CINTERFACE
#define COBJMACROS

#include <vector>

#include "WebView2.h"

#include "base/AbstractWebWidget.hpp"
#include "extra/WebView2EventHandler.hpp"

/*
  The easy way to work with Edge WebView2 requires WIL. According to MS:
  "The Windows Implementation Libraries (WIL) is a header-only C++ library
  created to make life easier for developers on Windows through readable
  type-safe C++ interfaces for common Windows coding patterns."
  Unfortunately WIL is not compatible with the MinGW GCC. But because Edge
  WebView2 is a COM component, it can still be called using its C interface.

  https://github.com/microsoft/wil/issues/117
  https://github.com/jchv/webview2-in-mingw
  https://www.codeproject.com/Articles/13601/COM-in-plain-C
*/

START_NAMESPACE_DISTRHO

class InternalWebView2EventHandler;

class EdgeWebWidget : public AbstractWebWidget, edge::WebView2EventHandler
{
public:
    EdgeWebWidget(Window& windowToMapTo);
    ~EdgeWebWidget();

    void onDisplay() override;
    void onResize(const ResizeEvent& ev) override;

    void setBackgroundColor(uint32_t rgba) override;
    void reparent(Window& windowToMapTo) override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

    // WebView2EventHandler

    HRESULT handleWebView2EnvironmentCompleted(HRESULT result,
                                    ICoreWebView2Environment* environment) override;
    HRESULT handleWebView2ControllerCompleted(HRESULT result,
                                    ICoreWebView2Controller* controller) override;
    HRESULT handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                    ICoreWebView2NavigationCompletedEventArgs *eventArgs) override;
    HRESULT handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                    ICoreWebView2WebMessageReceivedEventArgs *eventArgs) override;

private:
    inline void updateWebViewSize(Size<uint> size);

    void startWebViewInit();

    void webViewLoaderErrorMessageBox(HRESULT result);

    WNDCLASS            fHelperClass;
    HWND                fHelperHwnd;
    bool                fDisplayed;
    uint32_t            fBackgroundColor;
    std::vector<String> fInjectedScripts;
    String              fUrl;
    
    InternalWebView2EventHandler* fHandler;
    ICoreWebView2Controller*      fController;
    ICoreWebView2*                fView;

};

typedef EdgeWebWidget PlatformWebWidget;


// The event handler lifetime cannot be bound to its owner lifetime, otherwise
// the Edge WebView2 could callback a deleted object. That would happen for
// example if the plugin UI is opened and suddenly closed before web content
// finishes loading, or before WebView2 has fully initialized itself.

class InternalWebView2EventHandler : public edge::WebView2EventHandler {
public:
    InternalWebView2EventHandler(edge::WebView2EventHandler* ownerRef)
        : fOwnerWeakRef(ownerRef)
    {
        incRefCount();
    }

    void release()
    {
        fOwnerWeakRef = 0;
        if (decRefCount() == 0) {
            delete this;
        }
    }

    HRESULT handleWebView2EnvironmentCompleted(HRESULT result,
                                    ICoreWebView2Environment* environment) override
    {
        if (fOwnerWeakRef != 0) {
            return fOwnerWeakRef->handleWebView2EnvironmentCompleted(result, environment);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2ControllerCompleted(HRESULT result,
                                    ICoreWebView2Controller* controller) override
    {
        if (fOwnerWeakRef != 0) {
            return fOwnerWeakRef->handleWebView2ControllerCompleted(result, controller);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                    ICoreWebView2NavigationCompletedEventArgs *eventArgs) override
    {
        if (fOwnerWeakRef != 0) {
            return fOwnerWeakRef->handleWebView2NavigationCompleted(sender, eventArgs);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                    ICoreWebView2WebMessageReceivedEventArgs *eventArgs) override
    {
        if (fOwnerWeakRef != 0) {
            return fOwnerWeakRef->handleWebView2WebMessageReceived(sender, eventArgs);
        } else {
            return E_ABORT;
        }
    }

private:
    edge::WebView2EventHandler* fOwnerWeakRef;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBWIDGET_HPP
