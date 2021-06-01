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

#include <string>

#include "WebView2.h"

#include "WebViewInterface.hpp"
#include "WebViewHandler.hpp"

START_NAMESPACE_DISTRHO

class EdgeWebView : public WebViewInterface, edge::WebViewHandler
{
public:
    EdgeWebView();
    ~EdgeWebView();

    void navigate(String url) override;
    void reparent(uintptr_t windowId) override;
    void resize(const Size<uint>& size) override;

    // WebViewHandler

    HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                              ICoreWebView2Environment* environment) override;
    HRESULT handleWebViewControllerCompleted(HRESULT result,
                                             ICoreWebView2Controller* controller) override;
    HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                             ICoreWebView2NavigationCompletedEventArgs *eventArgs) override;

private:
    void errorMessageBox(std::wstring message, HRESULT result);

    ICoreWebView2Controller* fController;

    String     fUrl;
    uintptr_t  fWindowId;
    Size<uint> fSize;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEW_HPP
