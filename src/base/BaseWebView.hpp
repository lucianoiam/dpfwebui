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
#include <deque>

#include "dgl/Geometry.hpp"
#include "extra/String.hpp"

#include "ScriptValue.hpp"

#define SAFE_GET_VALUE(d)   (d.empty() ? ScriptValue() : d.front())
#define SAFE_GET_BOOL(d)    SAFE_GET_VALUE(d).asBool()
#define SAFE_GET_DOUBLE(d)  SAFE_GET_VALUE(d).asDouble()
#define SAFE_GET_STRING(d)  SAFE_GET_VALUE(d).asString()
#define SAFE_POP_VALUE(d)   {if (!d.empty()) d.pop_front();}

START_NAMESPACE_DISTRHO

typedef std::deque<ScriptValue> ScriptValueDeque;

class WebViewEventHandler
{
public:
    virtual void webViewLoadFinished() = 0;
    virtual bool webViewScriptMessageReceived(ScriptValueDeque& args) = 0;

};

class BaseWebView
{
public:
    BaseWebView(WebViewEventHandler& handler) : fHandler(handler) {}
    virtual ~BaseWebView() {};

    virtual void reparent(uintptr_t windowId) = 0;
    virtual void resize(const DGL::Size<uint>& size) = 0;
    virtual void navigate(String url) = 0;
    virtual void runScript(String source) = 0;
    virtual void injectScript(String source) = 0;

    void sendScriptMessage(ScriptValueDeque& args);

protected:
    void injectDefaultScripts(String platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(ScriptValueDeque& args);

private:
    void addStylesheet(String source);

    WebViewEventHandler& fHandler;

};

END_NAMESPACE_DISTRHO

#endif // BASEWEBVIEW_HPP
