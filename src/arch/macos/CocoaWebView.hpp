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

#ifndef COCOAWEBVIEW_HPP
#define COCOAWEBVIEW_HPP

#include "base/BaseWebView.hpp"

// While it is possible to #import Obj-C headers here, that would force all
// source files importing CocoaWebView.hpp to do so before any other project
// headers to avoid symbol name collisions. Do not make any assumption.

#define WEBVIEW_CLASS CocoaWebView

START_NAMESPACE_DISTRHO

class CocoaWebView : public BaseWebView
{
public:
    CocoaWebView(WebViewScriptMessageHandler& handler);
    ~CocoaWebView();

    void reparent(uintptr_t windowId) override;
    void resize(const DGL::Size<uint>& size) override;
    void navigate(String url) override;
    void runScript(String source) override;
    void injectScript(String source) override;
    
    // Allow calling some protected methods from the ObjC WKNavigationDelegate
    
    void didFinishNavigation() { loadFinished(); }
    void didReceiveScriptMessage(ScriptMessageArguments& args) { handleWebViewScriptMessage(args); }

private:
    void *fView;
    void *fDelegate;

};

END_NAMESPACE_DISTRHO

#endif  // COCOAWEBVIEW_HPP