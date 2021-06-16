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

#ifndef BASEWEBVIEW_HPP
#define BASEWEBVIEW_HPP

#include <cstdint>
#include <vector>

#include "dgl/TopLevelWidget.hpp"
#include "dgl/Geometry.hpp"
#include "extra/String.hpp"
#include "Window.hpp"

#include "ScriptValue.hpp"

START_NAMESPACE_DISTRHO

typedef std::vector<ScriptValue> ScriptValueVector;

class WebViewEventHandler
{
public:
    virtual void handleWebViewLoadFinished() = 0;
    virtual void handleWebViewScriptMessageReceived(const ScriptValueVector& args) = 0;

};

class BaseWebView : public TopLevelWidget
{
public:
    BaseWebView(Window& windowToMapTo, WebViewEventHandler& handler);
    virtual ~BaseWebView() {};

    void onDisplay() override {};

    virtual void setBackgroundColor(uint32_t rgba) = 0;
    virtual void reparent(Window& windowToMapTo) = 0;
    virtual void resize(const DGL::Size<uint>& size) = 0;
    virtual void navigate(String& url) = 0;
    virtual void runScript(String& source) = 0;
    virtual void injectScript(String& source) = 0;
    virtual void start() {};
    
    void postMessage(const ScriptValueVector& args);

protected:
    void injectDefaultScripts(String& platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(const ScriptValueVector& args);

private:
    String serializeScriptValues(const ScriptValueVector& args);

    void addStylesheet(String& source);

    WebViewEventHandler& fHandler;

};

END_NAMESPACE_DISTRHO

#endif // BASEWEBVIEW_HPP
