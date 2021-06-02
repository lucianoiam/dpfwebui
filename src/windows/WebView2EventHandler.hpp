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

#ifndef WEBVIEW2EVENTHANDLER_HPP
#define WEBVIEW2EVENTHANDLER_HPP

/*
  The easy way to work with WebView2 requires WIL - "Windows Implementation Library"
  which is not compatible with MinGW, see https://github.com/microsoft/wil/issues/117
  
  Solution is to use the C interface to WebView2 as shown here:
  https://github.com/jchv/webview2-in-mingw
*/

#define UNICODE
#define CINTERFACE

#include "WebView2.h"

namespace edge {

class WebView2EventHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                             public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                             public ICoreWebView2NavigationCompletedEventHandler
{
public:
    WebView2EventHandler();
    virtual ~WebView2EventHandler() {};

    virtual HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                                      ICoreWebView2Environment* environment)
    {
        (void)result;
        (void)environment;
        return S_OK;
    }

    virtual HRESULT handleWebViewControllerCompleted(HRESULT result,
                                                     ICoreWebView2Controller* controller)
    {
        (void)result;
        (void)controller;
        return S_OK;
    }

    virtual HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                                     ICoreWebView2NavigationCompletedEventArgs *eventArgs)
    {
        (void)sender;
        (void)eventArgs;
        return S_OK;
    }

};

} // namespace edge

#endif // WEBVIEW2EVENTHANDLER_HPP
