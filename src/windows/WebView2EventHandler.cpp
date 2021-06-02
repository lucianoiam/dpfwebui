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

#include "WebView2EventHandler.hpp"
#include "com.hpp"

using namespace edge;
using namespace com;


// EnvironmentCompleted

static HRESULT STDMETHODCALLTYPE Impl_EnvironmentCompletedHandler(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This,
    HRESULT result, ICoreWebView2Environment* environment)
{
    return static_cast<WebView2EventHandler*>(This)->handleWebViewEnvironmentCompleted(result, environment);
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl Vtbl_EnvironmentCompletedHandler = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    Impl_EnvironmentCompletedHandler,
};


// ControllerCompleted

static HRESULT STDMETHODCALLTYPE Impl_ControllerCompletedHandler(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This,
    HRESULT result, ICoreWebView2Controller* controller)
{
    return static_cast<WebView2EventHandler*>(This)->handleWebViewControllerCompleted(result, controller);
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl Vtbl_ControllerCompletedHandler = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    Impl_ControllerCompletedHandler,
};


// NavigationCompletedEvent

static HRESULT STDMETHODCALLTYPE Impl_NavigationCompletedEventHandler(
    ICoreWebView2NavigationCompletedEventHandler* This,
    ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)
{
    return static_cast<WebView2EventHandler*>(This)->handleWebViewNavigationCompleted(sender, args);
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl Vtbl_NavigationCompletedEventHandler = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    Impl_NavigationCompletedEventHandler,
};


WebView2EventHandler::WebView2EventHandler()
    : ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&Vtbl_EnvironmentCompletedHandler}
    , ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&Vtbl_ControllerCompletedHandler}
    , ICoreWebView2NavigationCompletedEventHandler{&Vtbl_NavigationCompletedEventHandler}
{}
