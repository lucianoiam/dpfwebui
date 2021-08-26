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

#include "dgl/SubWidget.hpp"
#include "dgl/Geometry.hpp"
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
    AbstractWebView(uintptr_t parentWindowHandle)
        : fParentWindowHandle(parentWindowHandle)
        , fKeyboardFocus(false)
        , fPrintTraffic(false) {}
    virtual ~AbstractWebView() {}

    virtual void setBackgroundColor(uint32_t rgba) = 0;
    virtual void setSize(uint width, uint height) = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;
    
    //virtual bool onKeyboard(uint mod, uint flags, uint time, bool press, uint key, uint keycode) = 0;
    virtual void setKeyboardFocus(bool focus) { fKeyboardFocus = focus; }
    bool         isKeyboardFocus() { return fKeyboardFocus; }

    void setPrintTraffic(bool printTraffic) { fPrintTraffic = printTraffic; }
    void setEventHandler(WebViewEventHandler* handler) { fHandler = handler; }
    void postMessage(const JsValueVector& args);

protected:
    uintptr_t getParentWindowHandle() { return fParentWindowHandle; }

    void injectDefaultScripts(String& platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(const JsValueVector& args);

private:
    String serializeJsValues(const JsValueVector& args);

    void addStylesheet(String& source);

    uintptr_t fParentWindowHandle;
    bool      fKeyboardFocus;
    bool      fPrintTraffic;

    WebViewEventHandler* fHandler;

};

END_NAMESPACE_DISTRHO

#endif // ABSTRACT_WEBVIEW_HPP
