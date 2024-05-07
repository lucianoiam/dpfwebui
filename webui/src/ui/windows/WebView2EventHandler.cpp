/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

using namespace edge;

#define pInstance static_cast<WebView2EventHandler*>(This)


template <typename T>
static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject)
{
    (void)This;
    (void)riid;
    (void)ppvObject;
    return E_NOINTERFACE;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Impl_AddRef(T* This)
{
    return pInstance->incRefCount();
}

template <typename T>
static ULONG STDMETHODCALLTYPE Impl_Release(T* This)
{
    int refCount = pInstance->decRefCount();
    if (refCount == 0) {
        delete pInstance;
    }
    return refCount;
}


// EnvironmentCompleted

static HRESULT STDMETHODCALLTYPE Impl_EnvironmentCompletedHandler(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This,
    HRESULT result, ICoreWebView2Environment* environment)
{
    return pInstance->handleWebView2EnvironmentCompleted(result, environment);
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl Vtbl_EnvironmentCompletedHandler = {
    Null_QueryInterface,
    Impl_AddRef,
    Impl_Release,
    Impl_EnvironmentCompletedHandler,
};


// ControllerCompleted

static HRESULT STDMETHODCALLTYPE Impl_ControllerCompletedHandler(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This,
    HRESULT result, ICoreWebView2Controller* controller)
{
    return pInstance->handleWebView2ControllerCompleted(result, controller);
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl Vtbl_ControllerCompletedHandler = {
    Null_QueryInterface,
    Impl_AddRef,
    Impl_Release,
    Impl_ControllerCompletedHandler,
};


// NavigationCompletedEvent

static HRESULT STDMETHODCALLTYPE Impl_NavigationCompletedEventHandler(
    ICoreWebView2NavigationCompletedEventHandler* This,
    ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    return pInstance->handleWebView2NavigationCompleted(sender, eventArgs);
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl Vtbl_NavigationCompletedEventHandler = {
    Null_QueryInterface,
    Impl_AddRef,
    Impl_Release,
    Impl_NavigationCompletedEventHandler,
};


// WebMessageReceivedEvent

static HRESULT STDMETHODCALLTYPE Impl_WebMessageReceivedEventHandler(
    ICoreWebView2WebMessageReceivedEventHandler* This,
    ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    return pInstance->handleWebView2WebMessageReceived(sender, eventArgs);
}

static ICoreWebView2WebMessageReceivedEventHandlerVtbl Vtbl_WebMessageReceivedEventHandler = {
    Null_QueryInterface,
    Impl_AddRef,
    Impl_Release,
    Impl_WebMessageReceivedEventHandler,
};


WebView2EventHandler::WebView2EventHandler()
    : ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler { &Vtbl_EnvironmentCompletedHandler }
    , ICoreWebView2CreateCoreWebView2ControllerCompletedHandler { &Vtbl_ControllerCompletedHandler }
    , ICoreWebView2NavigationCompletedEventHandler { &Vtbl_NavigationCompletedEventHandler }
    , ICoreWebView2WebMessageReceivedEventHandler { &Vtbl_WebMessageReceivedEventHandler }
    , fRefCount(0)
{}
