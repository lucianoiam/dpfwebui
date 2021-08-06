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

#ifndef WEBVIEW2_EVENT_HANDLER_HPP
#define WEBVIEW2_EVENT_HANDLER_HPP

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
    virtual ~WebView2EventHandler() {}

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

#endif // WEBVIEW2_EVENT_HANDLER_HPP
