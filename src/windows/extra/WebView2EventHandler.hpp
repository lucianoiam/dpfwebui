/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
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

#ifndef WEBVIEW2EVENTHANDLER_HPP
#define WEBVIEW2EVENTHANDLER_HPP

#define UNICODE
#define CINTERFACE

#include "WebView2.h"

namespace edge {

class WebView2EventHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                             public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                             public ICoreWebView2NavigationCompletedEventHandler,
                             public ICoreWebView2WebMessageReceivedEventHandler
{
public:
    WebView2EventHandler();
    virtual ~WebView2EventHandler() {};

    virtual HRESULT handleWebView2EnvironmentCompleted(HRESULT result,
                                                       ICoreWebView2Environment* environment)
    {
        (void)result;
        (void)environment;
        return S_OK;
    }

    virtual HRESULT handleWebView2ControllerCompleted(HRESULT result,
                                                      ICoreWebView2Controller* controller)
    {
        (void)result;
        (void)controller;
        return S_OK;
    }

    virtual HRESULT handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                                      ICoreWebView2NavigationCompletedEventArgs *eventArgs)
    {
        (void)sender;
        (void)eventArgs;
        return S_OK;
    }

    virtual HRESULT handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                     ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
    {
        (void)sender;
        (void)eventArgs;
        return S_OK;
    }

    int incRefCount() { return ++fRefCount; }
    int decRefCount() { return fRefCount == 0 ? 0 : --fRefCount; }

private:
    int fRefCount;

};

} // namespace edge

#endif // WEBVIEW2EVENTHANDLER_HPP
