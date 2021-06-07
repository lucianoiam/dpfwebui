/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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
