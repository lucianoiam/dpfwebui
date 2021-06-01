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

#include "WebViewHandler.hpp"

using namespace webview;

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

static ICoreWebView2NavigationCompletedEventHandlerVtbl EventInterfacesNavigationCompletedEventHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfacesNavigationCompletedEvent_Invoke,
};


// __EVENT__

/*static HRESULT STDMETHODCALLTYPE EventInterfaces__EVENT___Invoke(
    ICoreWebView2__EVENT__Handler* This, HRESULT result,
    ICoreWebView2Controller* controller)
{
    return static_cast<WebViewHandler*>(This)->handleWebView__EVENT__(result, controller);
}

static ICoreWebView2__EVENT__HandlerVtbl EventInterfaces__EVENT__HandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces__EVENT___Invoke,
};*/


WebViewHandler::WebViewHandler()
    : ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&EventInterfaces_EnvironmentCompletedHandlerVtbl}
    , ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&EventInterfaces_ControllerCompletedHandlerVtbl}
    , ICoreWebView2NavigationCompletedEventHandler{&EventInterfacesNavigationCompletedEventHandlerVtbl}
    //,ICoreWebView2__EVENT__Handler{&EventInterfaces__EVENT__HandlerVtbl}
{}
