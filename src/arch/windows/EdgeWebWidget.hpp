/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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
  WebView2 is a COM component, it can still be integrated using its C interface.

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
    void initWebView();
    void updateWebViewBounds();
    
    void webViewLoaderErrorMessageBox(HRESULT result);

    WNDCLASS            fInitHelperClass;
    HWND                fInitHelperHwnd;
    bool                fDisplayed;
    uint32_t            fBackgroundColor;
    std::vector<String> fInjectedScripts;
    String              fUrl;
    
    InternalWebView2EventHandler* fHandler;
    ICoreWebView2Controller*      fController;
    ICoreWebView2*                fView;

};


// The event handler lifetime cannot be bound to its owner lifetime, otherwise
// the Edge WebView2 could callback a deleted object. That would happen for
// example if the widget is created and suddenly destroyed before web content
// finishes loading, or before WebView2 has fully initialized itself.
// In the case of ProxyWebUI the scenario is easily reproducible by opening the
// plugin window on Carla and immediately closing it before the web UI shows up.
// Note that InternalWebView2EventHandler is not fully COM compliant, it is
// lacking the query interface method. It would also need to be registered for
// allowing instantiation with CoCreateInstance() but we do not need all that
// boilerplate, just the bare minimum to make Edge WebView2 happy and get
// notified of events in return. The handler class is expected to be init'd
// using the C++ new operator and disposed of by calling its release() method.

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
