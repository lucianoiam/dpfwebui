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

#ifndef COCOA_WEBVIEW_HPP
#define COCOA_WEBVIEW_HPP

#include "../WebViewBase.hpp"

// While it is possible to #import Obj-C headers here, that would force all
// source files importing CocoaWebView.hpp to do so before any other project
// headers to avoid symbol name collisions. Do not make any assumption.

START_NAMESPACE_DISTRHO

class CocoaWebView : public WebViewBase
{
public:
    CocoaWebView(String userAgentComponent = String());
    virtual ~CocoaWebView();
    
    float getDevicePixelRatio() override;
    
    void realize() override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

    // Allow calling some protected methods from Objective-C instances
    
    void didFinishNavigation() { handleLoadFinished(); }
    void didReceiveScriptMessage(const Variant& payload) { handleScriptMessage(payload); }

protected:
    void onSize(uint width, uint height) override;
    void onSetParent(uintptr_t parent) override;

private:
    void* fBackground;
    void* fWebView;
    void* fDelegate;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CocoaWebView)

};

END_NAMESPACE_DISTRHO

#endif  // COCOA_WEBVIEW_HPP
