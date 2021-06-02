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

#include "WebView2Handler.hpp"
#include "WinApiStub.hpp"

using namespace edge;
using namespace winstub;


// EnvironmentCompleted

static HRESULT STDMETHODCALLTYPE EventInterfaces_EnvironmentCompleted_Invoke(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT result,
    ICoreWebView2Environment* created_environment)
{
    return static_cast<WebView2Handler*>(This)->handleWebViewEnvironmentCompleted(result, created_environment);
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
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This,
    HRESULT result, ICoreWebView2Controller* controller)
{
    return static_cast<WebView2Handler*>(This)->handleWebViewControllerCompleted(result, controller);
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
    return static_cast<WebView2Handler*>(This)->handleWebViewNavigationCompleted(sender, args);
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
    return static_cast<WebView2Handler*>(This)->handleWebView[ EVENT ]([ ARGS ]);
}

static ICoreWebView2[ EVENT ]HandlerVtbl EventInterfaces_[ EVENT ]HandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces[ EVENT ]_Invoke,
};*/


WebView2Handler::WebView2Handler()
    : ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&EventInterfaces_EnvironmentCompletedHandlerVtbl}
    , ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&EventInterfaces_ControllerCompletedHandlerVtbl}
    , ICoreWebView2NavigationCompletedEventHandler{&EventInterfaces_NavigationCompletedEventHandlerVtbl}
    //,ICoreWebView2[ EVENT ]Handler{&EventInterfaces_[ EVENT ]HandlerVtbl}
{}
