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

#include "BaseWebView.hpp"

#include <iostream>

#define JS_CONSOLE_OUTPUT       "window.console = {log: (s) => window.webviewHost.postMessage(['console.log', String(s)])};"
#define JS_DISABLE_CONTEXT_MENU "window.oncontextmenu = (e) => e.preventDefault();"
#define CSS_DISABLE_PINCH_ZOOM  "body { touch-action: pan-x pan-y; }"
#define CSS_DISABLE_SELECTION   "body { user-select: none; -webkit-user-select: none; }"

void BaseWebView::injectDefaultScripts()
{
    // Injected scripts run before any user script starts running
    injectScript(String(JS_CONSOLE_OUTPUT));
    injectScript(String(JS_DISABLE_CONTEXT_MENU));
}

void BaseWebView::loadFinished()
{
    // User scripts may have started running already
    addStylesheet(String(CSS_DISABLE_PINCH_ZOOM));
    addStylesheet(String(CSS_DISABLE_SELECTION));
}

void BaseWebView::handleWebViewScriptMessage(ScriptMessageArguments& args)
{
    if (GET_STRING_ARGUMENT(args) == "console.log") {
        POP_ARGUMENT(args);
        std::cerr << GET_STRING_ARGUMENT(args) << std::endl;
    } else {
        fHandler.handleWebViewScriptMessage(args);
    }
}

void BaseWebView::addStylesheet(String source)
{
    String js;
    js += "document.head.insertAdjacentHTML('beforeend', '<style>" + source + "</style>');";
    runScript(js);
}