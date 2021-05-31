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
