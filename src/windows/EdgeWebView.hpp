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

#ifndef EDGEWEBVIEW_HPP
#define EDGEWEBVIEW_HPP

#define UNICODE
#define CINTERFACE
#define COBJMACROS

#include <string>

#include "WebView2.h"

#include "../common/BaseWebView.hpp"
#include "WebView2EventHandler.hpp"

/*
  The easy way to work with WebView2 requires WIL - "Windows Implementation Library"
  which is not compatible with MinGW, see https://github.com/microsoft/wil/issues/117
  
  Solution is to use the C interface to WebView2
  
  https://github.com/jchv/webview2-in-mingw
  https://www.codeproject.com/Articles/13601/COM-in-plain-C
*/

#define WEBVIEW_CLASS EdgeWebView

START_NAMESPACE_DISTRHO

class EdgeWebView : public BaseWebView, edge::WebView2EventHandler
{
public:
    EdgeWebView(WebViewScriptMessageHandler& handler);
    ~EdgeWebView();

    void reparent(uintptr_t windowId) override;
    void resize(const Size<uint>& size) override;
    void navigate(String url) override;
    void runScript(String source) override;
    void injectScript(String source) override;

    // WebView2EventHandler

    HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                              ICoreWebView2Environment* environment) override;
    HRESULT handleWebViewControllerCompleted(HRESULT result,
                                             ICoreWebView2Controller* controller) override;
    HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                             ICoreWebView2NavigationCompletedEventArgs *eventArgs) override;

private:
    void errorMessageBox(std::wstring message, HRESULT result);

    ICoreWebView2Controller* fController;
    ICoreWebView2*           fView;

    uintptr_t  fWindowId;
    Size<uint> fSize;
    String     fUrl;
    String     fInjectJs;
    WNDCLASS   fHelperClass;
    HWND       fHelperHwnd;
    
};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEW_HPP
