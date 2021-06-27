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

#include "AbstractWebWidget.hpp"

#include <iostream>
#include <sstream>

#define JS_DISABLE_CONTEXT_MENU  "window.oncontextmenu = (e) => e.preventDefault();"
#define JS_CREATE_CONSOLE        "window.console = {log: (s) => window.webviewHost.postMessage(['console.log', String(s)])};"
#define JS_CREATE_HOST_OBJECT    "window.webviewHost = new EventTarget;" \
                                 "window.webviewHost.addMessageListener = (lr) => {" \
                                    "window.webviewHost.addEventListener('message', (ev) => lr(ev.detail))" \
                                 "};"
#define CSS_DISABLE_IMAGE_DRAG   "img { user-drag: none; -webkit-user-drag: none; }"
#define CSS_DISABLE_SELECTION    "body { user-select: none; -webkit-user-select: none; }"
#define CSS_DISABLE_PINCH_ZOOM   "body { touch-action: pan-x pan-y; }"
#define CSS_DISABLE_OVERFLOW     "body { overflow: hidden; }"

/**
   Keep this class generic; specific plugin features belong to ProxyWebUI.
 */

USE_NAMESPACE_DISTRHO

void AbstractWebWidget::injectDefaultScripts(String& platformSpecificScript)
{
    String js = String()
        + String(JS_DISABLE_CONTEXT_MENU)
        + String(JS_CREATE_CONSOLE)
        + String(JS_CREATE_HOST_OBJECT)
        + platformSpecificScript
    ;
    injectScript(js);
}

void AbstractWebWidget::handleLoadFinished()
{
    String css = String()
        + String(CSS_DISABLE_IMAGE_DRAG)
        + String(CSS_DISABLE_SELECTION)
        + String(CSS_DISABLE_PINCH_ZOOM)
        + String(CSS_DISABLE_OVERFLOW)
    ;
    addStylesheet(css);

    if (fHandler != 0) {
        fHandler->handleWebWidgetContentLoadFinished();
    }
}

void AbstractWebWidget::postMessage(const ScriptValueVector& args)
{
    // WebKit-based webviews implement a standard mechanism for transferring messages from JS to the
    // native side, carrying a payload of JavaScript values that can be accessed through jsc_value_*
    // calls in WebKitGTK or automatically bridged to Objective-C objects in WKWebView. On Edge
    // WebView2 only JSON is available, see EdgeWebWidget::handleWebView2WebMessageReceived().
    // There is no equivalent inverse mechanism for passing messages from native to JS, other than
    // calling custom JavaScript using a function provided by webviews on all platforms.
    // This method implements something like a "reverse postMessage()" aiming to keep the bridge
    // symmetrical. Global window.webviewHost is an EventTarget that can be listened for messages.
    String payload = serializeScriptValues(args);
    if (fPrintTraffic) {
        std::cerr << "cpp -> js : " << payload.buffer() << std::endl;
    }
    String js = "window.webviewHost.dispatchEvent(new CustomEvent('message',{detail:" + payload + "}));";
    runScript(js);
}

void AbstractWebWidget::handleScriptMessage(const ScriptValueVector& args)
{
    if ((args.size() > 1) && (args[0].getString() == "console.log")) {
        std::cerr << args[1].getString().buffer() << std::endl;
    } else {
        if (fPrintTraffic) {
            std::cerr << "cpp <- js : " << serializeScriptValues(args).buffer() << std::endl;
        }
        if (fHandler != 0) {
            fHandler->handleWebWidgetScriptMessageReceived(args);
        }
    }
}

void AbstractWebWidget::handleKeyboardEvent(int arg0, int arg1, void* data)
{
    fHandler->handleWebWidgetKeyboardEvent(arg0, arg1, data);
}

String AbstractWebWidget::serializeScriptValues(const ScriptValueVector& args)
{
    std::stringstream ss;
    ss << '[';

    for (ScriptValueVector::const_iterator it = args.cbegin(); it != args.cend(); ++it) {
        if (it != args.cbegin()) {
            ss << ',';
        }
        ss << *it;
    }

    ss << ']';

    return String(ss.str().c_str());
}

void AbstractWebWidget::addStylesheet(String& source)
{
    String js = "document.head.insertAdjacentHTML('beforeend', '<style>" + source + "</style>');";
    runScript(js);
}
