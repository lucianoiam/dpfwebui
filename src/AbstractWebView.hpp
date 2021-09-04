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

#ifndef ABSTRACT_WEBVIEW_HPP
#define ABSTRACT_WEBVIEW_HPP

#include <cstdint>
#include <vector>

#include "extra/String.hpp"
#include "Window.hpp"

#include "JsValue.hpp"

START_NAMESPACE_DISTRHO

typedef std::vector<JsValue> JsValueVector;

class WebViewEventHandler
{
public:
    virtual void handleWebViewContentLoadFinished() = 0;
    virtual void handleWebViewScriptMessageReceived(const JsValueVector& args) = 0;

};

class AbstractWebView
{
public:
    AbstractWebView()
        : fBackgroundColor(0)
        , fParent(0)
        , fKeyboardFocus(false)
        , fPrintTraffic(false)
        , fHandler(0) {}
    virtual ~AbstractWebView() {}

    bool      getKeyboardFocus();
    void      setKeyboardFocus(bool focus);
    uint32_t  getBackgroundColor();
    void      setBackgroundColor(uint32_t rgba);
    uintptr_t getParent();
    void      setParent(uintptr_t parent);

    void setPrintTraffic(bool printTraffic) { fPrintTraffic = printTraffic; }
    void setEventHandler(WebViewEventHandler* handler) { fHandler = handler; }
    void postMessage(const JsValueVector& args);

    virtual void setSize(uint width, uint height) = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;

protected:
    virtual void onKeyboardFocus(bool focus) { (void)focus; };
    virtual void onBackgroundColor(uint32_t rgba) { (void)rgba; };
    virtual void onParent(uintptr_t parent) { (void)parent; };

    void injectDefaultScripts(String& platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(const JsValueVector& args);

private:
    String serializeJsValues(const JsValueVector& args);

    void addStylesheet(String& source);

    uint32_t  fBackgroundColor;
    uintptr_t fParent;
    bool      fKeyboardFocus;
    bool      fPrintTraffic;

    WebViewEventHandler* fHandler;

};

END_NAMESPACE_DISTRHO

#endif // ABSTRACT_WEBVIEW_HPP
