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

#ifndef WEBVIEWHANDLER_HPP
#define WEBVIEWHANDLER_HPP

/*
  The official way to work with WebView2 requires WIL which is provided by the
  NuGet package Microsoft.Windows.ImplementationLibrary, but WIL is not
  compatible with MinGW. See https://github.com/microsoft/wil/issues/117
  
  Solution is to use the C interface to WebView2 as shown here:
  https://github.com/jchv/webview2-in-mingw
*/

#define CINTERFACE

#include "WebView2.h"

class WebViewHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                     public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                     public ICoreWebView2NavigationCompletedEventHandler
                     //public ICoreWebView2__EVENT__Handler
{
public:
    WebViewHandler();
    virtual ~WebViewHandler() {};

    virtual HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                                      ICoreWebView2Environment* environment) = 0;

    virtual HRESULT handleWebViewControllerCompleted(HRESULT result,
                                                     ICoreWebView2Controller* controller) = 0;

    virtual HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                                     ICoreWebView2NavigationCompletedEventArgs *eventArgs) = 0;

    //virtual HRESULT handleWebView__EVENT__( ... see WebView2.h for arguments ... ) = 0;

};

#endif // WEBVIEWHANDLER_HPP
