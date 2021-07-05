/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#ifndef ABSTRACTWEBWIDGET_HPP
#define ABSTRACTWEBWIDGET_HPP

#include <cstdint>
#include <vector>

#include "dgl/SubWidget.hpp"
#include "dgl/Geometry.hpp"
#include "extra/String.hpp"
#include "Window.hpp"

#include "ScriptValue.hpp"

START_NAMESPACE_DISTRHO

typedef std::vector<ScriptValue> ScriptValueVector;

class WebWidgetEventHandler
{
public:
    virtual void handleWebWidgetContentLoadFinished() = 0;
    virtual void handleWebWidgetScriptMessageReceived(const ScriptValueVector& args) = 0;

};

class AbstractWebWidget : public SubWidget
{
public:
    AbstractWebWidget(Widget *parentWidget) : SubWidget(parentWidget),
        fKeyboardFocus(false), fPrintTraffic(false) {}
    virtual ~AbstractWebWidget() {};

    virtual void setBackgroundColor(uint32_t rgba) = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;

    virtual void setKeyboardFocus(bool focus) { fKeyboardFocus = focus; }
    bool         isKeyboardFocus() { return fKeyboardFocus; }

    void setPrintTraffic(bool printTraffic) { fPrintTraffic = printTraffic; }
    void setEventHandler(WebWidgetEventHandler* handler) { fHandler = handler; }
    void postMessage(const ScriptValueVector& args);

protected:
    void onDisplay() override { /* no-op */ }
    
    void injectDefaultScripts(String& platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(const ScriptValueVector& args);

    bool fKeyboardFocus;

private:
    String serializeScriptValues(const ScriptValueVector& args);

    void addStylesheet(String& source);

    bool fPrintTraffic;

    WebWidgetEventHandler* fHandler;

};

END_NAMESPACE_DISTRHO

#endif // ABSTRACTWEBWIDGET_HPP
