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

#include "WebViewInterface.hpp"
#include "WebView2Handler.hpp"

START_NAMESPACE_DISTRHO

class EdgeWebView : public WebViewInterface, edge::WebView2Handler
{
public:
    EdgeWebView();
    ~EdgeWebView();

    void navigate(String url) override;
    void reparent(uintptr_t windowId) override;
    void resize(const Size<uint>& size) override;

    // WebViewHandler

    HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                              ICoreWebView2Environment* environment) override;
    HRESULT handleWebViewControllerCompleted(HRESULT result,
                                             ICoreWebView2Controller* controller) override;
    HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                             ICoreWebView2NavigationCompletedEventArgs *eventArgs) override;

private:
    void initWebView();
    void cleanupWebView();

    void errorMessageBox(std::wstring message, HRESULT result);

    ICoreWebView2Controller* fController;
    EventRegistrationToken   fEventToken;
    
    uintptr_t  fWindowId;
    String     fUrl;
    Size<uint> fSize;
    
};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEW_HPP
