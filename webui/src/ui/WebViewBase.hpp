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

#ifndef WEBVIEW_BASE_HPP
#define WEBVIEW_BASE_HPP

#include <cstdint>

#include "distrho/extra/String.hpp"
#include "Window.hpp"

#include "Variant.hpp"

START_NAMESPACE_DISTRHO

struct WebViewEventHandler
{
    virtual void handleWebViewLoadFinished() = 0;
    virtual void handleWebViewScriptMessage(const Variant& payload) = 0;
    virtual void handleWebViewConsole(const String& tag, const String& text) = 0;
};

class WebViewBase
{
public:
    WebViewBase(String userAgentComponent = String());
    virtual ~WebViewBase() {};

    uint getWidth();
    uint getHeight();
    void setSize(uint width, uint height);
    
    uint32_t getBackgroundColor();
    void     setBackgroundColor(uint32_t color);
    
    uintptr_t getParent();
    void      setParent(uintptr_t parent);
    
    bool getKeyboardFocus();
    void setKeyboardFocus(bool focus);

    void setPrintTraffic(bool printTraffic);
    void setEnvironmentBool(const char* key, bool value);
    void setEventHandler(WebViewEventHandler* handler);
    
    void postMessage(const Variant& payload);

    virtual float getDevicePixelRatio() = 0;
    
    virtual void realize() = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;

protected:
    virtual void onSize(uint width, uint height) = 0;
    virtual void onKeyboardFocus(bool focus) { (void)focus; };
    virtual void onSetParent(uintptr_t parent) { (void)parent; };

    void injectHostObjectScripts();
    
    void handleLoadFinished();
    void handleScriptMessage(const Variant& payload);

private:
    void addStylesheet(String& source);

    uint      fWidth;
    uint      fHeight;
    uint32_t  fBackgroundColor;
    uintptr_t fParent;
    bool      fKeyboardFocus;
    bool      fPrintTraffic;

    WebViewEventHandler* fHandler;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewBase)

};

END_NAMESPACE_DISTRHO

#endif // WEBVIEW_BASE_HPP
