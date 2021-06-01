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

#include "WebViewHandler.hpp"

using namespace edge;

template <typename T>
static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject)
{
    (void)This;
    (void)riid;
    (void)ppvObject;
    return E_NOINTERFACE;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_AddRef(T* This)
{
    (void)This;
    return 1;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_Release(T* This)
{
    (void)This;
    return 1;
}


// EnvironmentCompleted

static HRESULT STDMETHODCALLTYPE EventInterfaces_EnvironmentCompleted_Invoke(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT result,
    ICoreWebView2Environment* created_environment)
{
    return static_cast<WebViewHandler*>(This)->handleWebViewEnvironmentCompleted(result, created_environment);
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl EventInterfaces_EnvironmentCompletedHandlerVtbl =
{
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces_EnvironmentCompleted_Invoke,
};


// ControllerCompleted

static HRESULT STDMETHODCALLTYPE EventInterfaces_ControllerCompleted_Invoke(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, HRESULT result,
    ICoreWebView2Controller* controller)
{
    return static_cast<WebViewHandler*>(This)->handleWebViewControllerCompleted(result, controller);
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl EventInterfaces_ControllerCompletedHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces_ControllerCompleted_Invoke,
};


// NavigationCompletedEvent

static HRESULT STDMETHODCALLTYPE EventInterfacesNavigationCompletedEvent_Invoke(
    ICoreWebView2NavigationCompletedEventHandler* This,
    ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)
{
    return static_cast<WebViewHandler*>(This)->handleWebViewNavigationCompleted(sender, args);
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl EventInterfaces_NavigationCompletedEventHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfacesNavigationCompletedEvent_Invoke,
};


// [ EVENT ]

/*static HRESULT STDMETHODCALLTYPE EventInterfaces[ EVENT ]_Invoke(
    ICoreWebView2[ EVENT ]Handler* This,
    [ ARGS ])
{
    return static_cast<WebViewHandler*>(This)->handleWebView[ EVENT ]([ ARGS ]);
}

static ICoreWebView2[ EVENT ]HandlerVtbl EventInterfaces_[ EVENT ]HandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces[ EVENT ]_Invoke,
};*/


WebViewHandler::WebViewHandler()
    : ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&EventInterfaces_EnvironmentCompletedHandlerVtbl}
    , ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&EventInterfaces_ControllerCompletedHandlerVtbl}
    , ICoreWebView2NavigationCompletedEventHandler{&EventInterfaces_NavigationCompletedEventHandlerVtbl}
    //,ICoreWebView2[ EVENT ]Handler{&EventInterfaces_[ EVENT ]HandlerVtbl}
{}
