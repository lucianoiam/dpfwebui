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

#define GET_ARGUMENT(args)        (args.empty() ? ScriptValue() : args.front())
#define GET_BOOL_ARGUMENT(args)   GET_ARGUMENT(args).asBool()
#define GET_DOUBLE_ARGUMENT(args) GET_ARGUMENT(args).asDouble()
#define GET_STRING_ARGUMENT(args) GET_ARGUMENT(args).asString()
#define POP_ARGUMENT(args)        {if (!args.empty()) args.pop_front();}

START_NAMESPACE_DISTRHO

typedef std::deque<ScriptValue> ScriptMessageArguments;

class WebViewEventHandler
{
public:
	virtual void handleWebViewLoadFinished() = 0;
    virtual void handleWebViewScriptMessage(ScriptMessageArguments& args) = 0;

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

protected:
    void injectDefaultScripts(String platformSpecificScript);
    
    void handleLoadFinished();
    void handleScriptMessage(ScriptMessageArguments& args);

private:
    void addStylesheet(String source);

    WebViewEventHandler& fHandler;

};

END_NAMESPACE_DISTRHO

#endif // BASEWEBVIEW_HPP
