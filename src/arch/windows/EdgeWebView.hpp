/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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

#ifndef EDGEWEBVIEW_HPP
#define EDGEWEBVIEW_HPP

#define UNICODE
#define CINTERFACE
#define COBJMACROS

#include <vector>

#include "WebView2.h"

#include "base/BaseWebView.hpp"
#include "extra/WebView2EventHandler.hpp"

/*
  The easy way to work with WebView2 requires WIL - "Windows Implementation Library"
  which is not compatible with MinGW, see https://github.com/microsoft/wil/issues/117
  
  Solution is to use the C interface to WebView2
  
  https://github.com/jchv/webview2-in-mingw
  https://www.codeproject.com/Articles/13601/COM-in-plain-C
*/

#define WEBVIEW_CLASS EdgeWebView

START_NAMESPACE_DISTRHO

class EdgeWebViewInternalEventHandler;

class EdgeWebView : public BaseWebView, edge::WebView2EventHandler
{
public:
    EdgeWebView(WebViewEventHandler& handler);
    ~EdgeWebView();

    void start() override;
    void reparent(uintptr_t windowId) override;
    void resize(const Size<uint>& size) override;
    void navigate(String url) override;
    void runScript(String source) override;
    void injectScript(String source) override;

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
    void webViewLoaderErrorMessageBox(HRESULT result);

    WNDCLASS fHelperClass;
    HWND     fHelperHwnd;

    EdgeWebViewInternalEventHandler* fHandler;
    ICoreWebView2Controller*  fController;
    ICoreWebView2*            fView;

    // P means pending
    uintptr_t           fPWindowId;
    Size<uint>          fPSize;
    String              fPUrl;
    std::vector<String> fPInjectedScripts;

};


// The event handler lifetime cannot be bound to its owner lifetime, otherwise
// the Edge WebView2 could callback a deleted object. That would happen for
// example if the plugin UI is opened and suddenly closed before web content
// finishes loading, or before WebView2 has not fully initialized itself.

class EdgeWebViewInternalEventHandler : public edge::WebView2EventHandler {
public:
    EdgeWebViewInternalEventHandler(edge::WebView2EventHandler* weakOwner)
        : fWeakOwner(weakOwner)
    {
        incRefCount();
    }

    void release()
    {
        fWeakOwner = 0;
        if (decRefCount() == 0) {
            //DISTRHO_LOG_STDERR("RELEASE");
            delete this;
        }
    }

    HRESULT handleWebView2EnvironmentCompleted(HRESULT result,
                                    ICoreWebView2Environment* environment) override
    {
        if (fWeakOwner != 0) {
            return fWeakOwner->handleWebView2EnvironmentCompleted(result, environment);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2ControllerCompleted(HRESULT result,
                                    ICoreWebView2Controller* controller) override
    {
        if (fWeakOwner != 0) {
            return fWeakOwner->handleWebView2ControllerCompleted(result, controller);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                    ICoreWebView2NavigationCompletedEventArgs *eventArgs) override
    {
        if (fWeakOwner != 0) {
            return fWeakOwner->handleWebView2NavigationCompleted(sender, eventArgs);
        } else {
            return E_ABORT;
        }
    }

    HRESULT handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                    ICoreWebView2WebMessageReceivedEventArgs *eventArgs) override
    {
        if (fWeakOwner != 0) {
            return fWeakOwner->handleWebView2WebMessageReceived(sender, eventArgs);
        } else {
            return E_ABORT;
        }
    }

private:
    edge::WebView2EventHandler* fWeakOwner;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEW_HPP
