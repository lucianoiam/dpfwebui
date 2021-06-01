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

#ifndef EDGEWEBVIEWUI_HPP
#define EDGEWEBVIEWUI_HPP

#define UNICODE
#define CINTERFACE
#define COBJMACROS

#include <string>

#include "WebView2.h"

#include "WebViewHandler.hpp"
#include "WebUI.hpp"

START_NAMESPACE_DISTRHO

class EdgeWebViewUI : public WebUI, webview::WebViewHandler
{
public:
    EdgeWebViewUI();
    virtual ~EdgeWebViewUI();

    void parameterChanged(uint32_t index, float value) override;

    void reparent(uintptr_t windowId) override;

    // WebViewHandler

    HRESULT handleWebViewEnvironmentCompleted(HRESULT result,
                                              ICoreWebView2Environment* environment) override;
    HRESULT handleWebViewControllerCompleted(HRESULT result,
                                             ICoreWebView2Controller* controller) override;
    HRESULT handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                             ICoreWebView2NavigationCompletedEventArgs *eventArgs) override;
protected:
    void onResize(const ResizeEvent& ev) override;

private:
    void cleanupWebView();
    void resize();
    
    void errorMessageBox(std::wstring message, HRESULT result);

    ICoreWebView2Controller* fController;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEWUI_HPP
