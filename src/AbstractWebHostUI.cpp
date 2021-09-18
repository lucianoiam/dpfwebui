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

#include "AbstractWebHostUI.hpp"

#include "Path.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

AbstractWebHostUI::AbstractWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : UI(baseWidth, baseHeight)
    , fWebView(0)
    , fInitWidth(0)
    , fInitHeight(0)
    , fBackgroundColor(backgroundColor)
    , fFlushedInitMsgQueue(false)
    , fRunUiBlock(false)
{}

AbstractWebHostUI::~AbstractWebHostUI()
{
    if (fWebView != 0) {
        delete fWebView;
    }
}

bool AbstractWebHostUI::shouldCreateWebView()
{
    // When running as a plugin the UI ctor/dtor can be repeatedly called with
    // no parent window available, do not create the web view in such cases.
    return isStandalone() || (getParentWindowHandle() != 0);
}

AbstractWebView* AbstractWebHostUI::getWebView()
{
    return fWebView;
}

void AbstractWebHostUI::setWebView(AbstractWebView* webView)
{
    fWebView = webView;

    // Web views adjust their contents following the system display scale factor,
    // adjust window size so it correctly wraps content on high density displays.
    // Cannot call virtual method createStandaloneWindow() from constructor.
    uintptr_t parent = isStandalone() ? createStandaloneWindow() : getParentWindowHandle();

    float k = getDisplayScaleFactor(parent);
    fInitWidth = k * getWidth();
    fInitHeight = k * getHeight();

    fWebView->setParent(parent);
    fWebView->setBackgroundColor(fBackgroundColor);
    fWebView->setSize(fInitWidth, fInitHeight);
    fWebView->realize();

    fWebView->setEventHandler(this);
#ifdef HIPHOP_PRINT_TRAFFIC
    fWebView->setPrintTraffic(true);
#endif

    String js = String(
#include "ui/distrho-ui.js.inc"
    );
    js += "const DISTRHO = Object.freeze({ UI: UI });" \
          "UI = null;";
    fWebView->injectScript(js);

    String url = "file://" + path::getLibraryPath() + "/ui/index.html";
    fWebView->navigate(url);

    setSize(fInitWidth, fInitHeight);
};

void AbstractWebHostUI::webViewPostMessage(const JsValueVector& args)
{
    if (fFlushedInitMsgQueue) {
        fWebView->postMessage(args);
    } else {
        fInitMsgQueue.push_back(args);
    }
}

void AbstractWebHostUI::flushInitMessageQueue()
{
    if (fFlushedInitMsgQueue) {
        return;
    }

    fFlushedInitMsgQueue = true;

    for (InitMessageQueue::iterator it = fInitMsgQueue.begin(); it != fInitMsgQueue.end(); ++it) {
        fWebView->postMessage(*it);
    }
    
    fInitMsgQueue.clear();
}

void AbstractWebHostUI::setKeyboardFocus(bool focus)
{
    fWebView->setKeyboardFocus(focus);
}

void AbstractWebHostUI::uiIdle()
{
    if (fRunUiBlock) {
        fRunUiBlock = false;
        fQueuedUiBlock();
    }

    if (isStandalone()) {
        processStandaloneEvents();
    }
}

void AbstractWebHostUI::sizeChanged(uint width, uint height)
{
    UI::sizeChanged(width, height);
    
    fWebView->setSize(width, height);
    webViewPostMessage({"UI", "sizeChanged", width, height});
}

void AbstractWebHostUI::parameterChanged(uint32_t index, float value)
{
    webViewPostMessage({"UI", "parameterChanged", index, value});
}

#if DISTRHO_PLUGIN_WANT_PROGRAMS

void AbstractWebHostUI::programLoaded(uint32_t index)
{
    webViewPostMessage({"UI", "programLoaded", index});
}

#endif // DISTRHO_PLUGIN_WANT_PROGRAMS

#if DISTRHO_PLUGIN_WANT_STATE

void AbstractWebHostUI::stateChanged(const char* key, const char* value)
{
    webViewPostMessage({"UI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE

void AbstractWebHostUI::queue(const UiBlock& block)
{
    fQueuedUiBlock = block;
    fRunUiBlock = true;
}

void AbstractWebHostUI::handleWebViewContentLoadFinished()
{
    // Trigger the JavaScript sizeChanged() callback, useful for WKGTKRESIZEBUG.
    sizeChanged(fInitWidth, fInitHeight);
    onWebContentReady();
}

#define kArg0 2
#define kArg1 3
#define kArg2 4

void AbstractWebHostUI::handleWebViewScriptMessageReceived(const JsValueVector& args)
{
    if (args[0].getString() != "UI") {
        onWebMessageReceived(args); // passthrough
        return;
    }

    // It is not possible to implement JS synchronous calls that return values
    // without resorting to dirty hacks. Use JS async functions instead, and
    // fulfill their promises here. See for example isResizable() below.

    String method = args[1].getString();
    int argc = args.size() - kArg0;

    if (method == "getWidth") {
        webViewPostMessage({"UI", "getWidth", static_cast<double>(getWidth())});

    } else if (method == "getHeight") {
        webViewPostMessage({"UI", "getHeight", static_cast<double>(getHeight())});

    } else if ((method == "setWidth") && (argc == 1)) {
        queue([this, args]() {
            setWidth(static_cast<uint>(args[kArg0].getDouble()));
        });

    } else if ((method == "setHeight") && (argc == 1)) {
        queue([this, args]() {
            setHeight(static_cast<uint>(args[kArg0].getDouble()));
        });

    } else if (method == "isResizable") {
        webViewPostMessage({"UI", "isResizable", isResizable()});

    } else if ((method == "setSize") && (argc == 2)) {
        // Queuing is needed for REAPER on Linux and does no harm on others
        queue([this, args]() {
            setSize(
                static_cast<uint>(args[kArg0].getDouble()), // width
                static_cast<uint>(args[kArg1].getDouble())  // height
            );
        });

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    } else if ((method == "sendNote") && (argc == 3)) {
        sendNote(
            static_cast<uint8_t>(args[kArg0].getDouble()),  // channel
            static_cast<uint8_t>(args[kArg1].getDouble()),  // note
            static_cast<uint8_t>(args[kArg2].getDouble())   // velocity
        );
#endif

    } else if ((method == "editParameter") && (argc == 2)) {
        editParameter(
            static_cast<uint32_t>(args[kArg0].getDouble()), // index
            static_cast<bool>(args[kArg1].getBool())        // started
        );

    } else if ((method == "setParameterValue") && (argc == 2)) {
        setParameterValue(
            static_cast<uint32_t>(args[kArg0].getDouble()), // index
            static_cast<float>(args[kArg1].getDouble())     // value
        );

#if DISTRHO_PLUGIN_WANT_STATE
    } else if ((method == "setState") && (argc == 2)) {
        setState(
            args[kArg0].getString(), // key
            args[kArg1].getString()  // value
        );
#endif // DISTRHO_PLUGIN_WANT_STATE

    } else if (method == "isStandalone") {
        webViewPostMessage({"UI", "isStandalone", isStandalone()});

    } else if ((method == "setKeyboardFocus") && (argc == 1)) {
        setKeyboardFocus(static_cast<bool>(args[kArg0].getBool()));

    } else if ((method == "openSystemWebBrowser") && (argc == 1)) {
        String url = args[kArg0].getString();
        openSystemWebBrowser(url);

    } else if (method == "getInitWidth") {
        webViewPostMessage({"UI", "getInitWidth", static_cast<double>(getInitWidth())});

    } else if (method == "getInitHeight") {
        webViewPostMessage({"UI", "getInitHeight", static_cast<double>(getInitHeight())});

    } else if (method == "flushInitMessageQueue") {
        flushInitMessageQueue();

    } else {
        HIPHOP_LOG_STDERR_COLOR("Invalid call to WebHostUI method");
    }
}
