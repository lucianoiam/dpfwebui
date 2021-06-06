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

#include "dgl/Geometry.hpp"
#include "extra/String.hpp"

#include "ScriptValue.hpp"

START_NAMESPACE_DISTRHO

typedef std::vector<ScriptValue> ScriptMessageArguments;

class WebViewScriptMessageHandler
{
public:
    virtual void handleWebViewScriptMessage(const ScriptMessageArguments& args) = 0;

};

class BaseWebView
{
public:
    BaseWebView(WebViewScriptMessageHandler& handler) : fHandler(handler) {}
    virtual ~BaseWebView() {};

    virtual void reparent(uintptr_t windowId) = 0;
    virtual void resize(const DGL::Size<uint>& size) = 0;
    virtual void navigate(String url) = 0;
    virtual void runScript(String source) = 0;
    virtual void injectScript(String source) = 0;

protected:
    void createConsole();
    void loadFinished();
    void handleWebViewScriptMessage(const ScriptMessageArguments& args);

private:
    void addStylesheet(String source);

    WebViewScriptMessageHandler& fHandler;

};

END_NAMESPACE_DISTRHO

#endif // BASEWEBVIEW_HPP
