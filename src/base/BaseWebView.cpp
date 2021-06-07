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

#define JS_CREATE_HOST_OBJECT    "window.webviewHost = new EventTarget;"
#define JS_CREATE_CONSOLE        "window.console = {log: (s) => window.webviewHost.postMessage(['console.log', String(s)])};"
#define JS_DISABLE_CONTEXT_MENU  "window.oncontextmenu = (e) => e.preventDefault();"
#define CSS_DISABLE_PINCH_ZOOM   "body { touch-action: pan-x pan-y; }"
#define CSS_DISABLE_SELECTION    "body { user-select: none; -webkit-user-select: none; }"

void BaseWebView::injectDefaultScripts(String platformSpecificScript)
{
    injectScript(String(JS_CREATE_HOST_OBJECT));
    injectScript(String(JS_CREATE_CONSOLE));
    injectScript(String(JS_DISABLE_CONTEXT_MENU));
    injectScript(platformSpecificScript);
}

void BaseWebView::handleLoadFinished()
{
    // User scripts may have started running already
    addStylesheet(String(CSS_DISABLE_PINCH_ZOOM));
    addStylesheet(String(CSS_DISABLE_SELECTION));
    fHandler.webViewLoadFinished();
}

void BaseWebView::sendHostMessage(const ScriptValueVector& args)
{
	// WebKit-based webviews implement a mechanism for transferring messages from JS to the native
	// side, carrying a payload of JavaScript values that can be accessed through jsc_value_* calls
	// in Linux WebKitGTK or automatically bridged to its Objective-C counterparts in Mac WKWebView.
	// On Edge WebView2 only JSON is available (see EdgeWebView::handleWebView2WebMessageReceived).
	// There is no equivalent inverse mechanism for passing messages from native to JS, other than
	// calling custom JavaScript using a function provided by all webviews.
	// This method implements something like a "reverse" postMessage() aiming to keep things
	// symmetrical and avoiding custom JavaScript as much as possible while keeping things generic.
	// The window.webviewHost object is an EventTarget that can be listened for messages.

    // TODO
    runScript(String("window.webviewHost.dispatchEvent(new CustomEvent('message',{detail:456}));"));
}

void BaseWebView::handleScriptMessage(const ScriptValueVector& args)
{
    if ((args.size() > 1) && (args[0].getString() == "console.log")) {
        std::cerr << args[1].getString() << std::endl;
    } else {
        fHandler.webViewScriptMessageReceived(args);
    }
}

void BaseWebView::addStylesheet(String source)
{
    String js;
    js += "document.head.insertAdjacentHTML('beforeend', '<style>" + source + "</style>');";
    runScript(js);
}
