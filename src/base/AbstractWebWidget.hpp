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

#ifndef ABSTRACTWEBWIDGET_HPP
#define ABSTRACTWEBWIDGET_HPP

#include <cstdint>
#include <vector>

#include "dgl/TopLevelWidget.hpp"
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

class AbstractWebWidget : public TopLevelWidget
{
public:
    AbstractWebWidget(Window& windowToMapTo) : TopLevelWidget(windowToMapTo),
        fGrabKeyboardInput(false), fPrintTraffic(false) {}
    virtual ~AbstractWebWidget() {};

    void onDisplay() override { /* no-op */ }
    
    virtual void setBackgroundColor(uint32_t rgba) = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;

    bool isGrabKeyboardInput() { return fGrabKeyboardInput; }
    void setGrabKeyboardInput(bool grabKeyboardInput) { fGrabKeyboardInput = grabKeyboardInput; }
    void setPrintTraffic(bool printTraffic) { fPrintTraffic = printTraffic; }
    void setEventHandler(WebWidgetEventHandler* handler) { fHandler = handler; }
    void postMessage(const ScriptValueVector& args);

protected:
    void injectDefaultScripts(String& platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(const ScriptValueVector& args);

    bool fGrabKeyboardInput;

private:
    String serializeScriptValues(const ScriptValueVector& args);

    void addStylesheet(String& source);

    bool fPrintTraffic;

    WebWidgetEventHandler* fHandler;

};

END_NAMESPACE_DISTRHO

#endif // ABSTRACTWEBWIDGET_HPP
