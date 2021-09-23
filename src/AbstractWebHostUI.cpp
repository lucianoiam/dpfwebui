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
    , fUiBlockQueued(false)
    , fInitMessageQueueFlushed(false)
{
    // It is not possible to implement JS synchronous calls that return values
    // without resorting to dirty hacks. Use JS async functions instead, and
    // fulfill their promises here. See for example getWidth() and getHeight().

    fHandler["getWidth"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "getWidth", static_cast<double>(getWidth())});
    });

    fHandler["getHeight"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "getHeight", static_cast<double>(getHeight())});
    });

    fHandler["isResizable"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "isResizable", isResizable()});
    });

    // Some calls need to be run on next uiIdle() iteration to ensure they work
    // as expected on all platform/host combinations. This is specifically
    // needed for REAPER on Linux. Queuing adds a slight latency but that is
    // preferrable to giving special treatment to a certain host on a certain
    // platform. Special cases based on the host type are not supported, in fact
    // DPF does not have a method to query the host type.

    fHandler["setWidth"] = std::make_pair(1, [this](const JsValueVector& args) {
        queue([this, args]() {
            setWidth(static_cast<uint>(args[0].getDouble()));
        });
    });

    fHandler["setHeight"] = std::make_pair(1, [this](const JsValueVector& args) {
        queue([this, args]() {
            setHeight(static_cast<uint>(args[0].getDouble()));
        });
    });

    fHandler["setSize"] = std::make_pair(2, [this](const JsValueVector& args) {
        queue([this, args]() {
            setSize(
                static_cast<uint>(args[0].getDouble()), // width
                static_cast<uint>(args[1].getDouble())  // height
            );
        });
    });

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    fHandler["sendNote"] = std::make_pair(3, [this](const JsValueVector& args) {
        sendNote(
            static_cast<uint8_t>(args[0].getDouble()),  // channel
            static_cast<uint8_t>(args[1].getDouble()),  // note
            static_cast<uint8_t>(args[2].getDouble())   // velocity
        );
    });
#endif // DISTRHO_PLUGIN_WANT_MIDI_INPUT

    fHandler["editParameter"] = std::make_pair(2, [this](const JsValueVector& args) {
        editParameter(
            static_cast<uint32_t>(args[0].getDouble()), // index
            static_cast<bool>(args[1].getBool())        // started
        );
    });

    fHandler["setParameterValue"] = std::make_pair(2, [this](const JsValueVector& args) {
        setParameterValue(
            static_cast<uint32_t>(args[0].getDouble()), // index
            static_cast<float>(args[1].getDouble())     // value
        );
    });

#if DISTRHO_PLUGIN_WANT_STATE
    fHandler["setState"] = std::make_pair(2, [this](const JsValueVector& args) {
        setState(
            args[0].getString(), // key
            args[1].getString()  // value
        );
    });
#endif // DISTRHO_PLUGIN_WANT_STATE

    fHandler["isStandalone"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "isStandalone", isStandalone()});
    });

    fHandler["setKeyboardFocus"] = std::make_pair(1, [this](const JsValueVector& args) {
        setKeyboardFocus(static_cast<bool>(args[0].getBool()));
    });

    fHandler["openSystemWebBrowser"] = std::make_pair(1, [this](const JsValueVector& args) {
        String url = args[0].getString();
        openSystemWebBrowser(url);
    });

    fHandler["getInitWidth"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "getInitWidth", static_cast<double>(getInitWidth())});
    });

    fHandler["getInitHeight"] = std::make_pair(0, [this](const JsValueVector&) {
        webViewPostMessage({"UI", "getInitHeight", static_cast<double>(getInitHeight())});
    });

    fHandler["flushInitMessageQueue"] = std::make_pair(0, [this](const JsValueVector&) {
        flushInitMessageQueue();
    });
}

AbstractWebHostUI::~AbstractWebHostUI()
{
    if (fWebView != 0) {
        delete fWebView;
    }
}

void AbstractWebHostUI::queue(const UiBlock& block)
{
    fUiBlock = block;
    fUiBlockQueued = true;
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

    String url = "file://" + path::getLibraryPath() + "/ui/index.html";
    fWebView->navigate(url);

    setSize(fInitWidth, fInitHeight);
};

void AbstractWebHostUI::webViewPostMessage(const JsValueVector& args)
{
    if (fInitMessageQueueFlushed) {
        fWebView->postMessage(args);
    } else {
        fInitMessageQueue.push_back(args);
    }
}

void AbstractWebHostUI::flushInitMessageQueue()
{
    if (fInitMessageQueueFlushed) {
        return;
    }

    fInitMessageQueueFlushed = true;

    for (InitMessageQueue::iterator it = fInitMessageQueue.begin(); it != fInitMessageQueue.end(); ++it) {
        fWebView->postMessage(*it);
    }
    
    fInitMessageQueue.clear();
}

void AbstractWebHostUI::setKeyboardFocus(bool focus)
{
    fWebView->setKeyboardFocus(focus);
}

void AbstractWebHostUI::uiIdle()
{
    if (fUiBlockQueued) {
        fUiBlockQueued = false;
        fUiBlock();
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

void AbstractWebHostUI::handleWebViewContentLoadFinished()
{
    // Trigger the JavaScript sizeChanged() callback, useful for WKGTKRESIZEBUG.
    sizeChanged(fInitWidth, fInitHeight);
    onWebContentReady();
}

void AbstractWebHostUI::handleWebViewScriptMessageReceived(const JsValueVector& args)
{
    if ((args.size() < 2) || (args[0].getString() != "UI")) {
        onWebMessageReceived(args); // passthrough
        return;
    }

    String key = args[1].getString();

    if (fHandler.find(key.buffer()) == fHandler.end()) {
        HIPHOP_LOG_STDERR_COLOR("Unknown WebHostUI method");
        return;
    }

    const JsValueVector handlerArgs(args.cbegin() + 2, args.cend());
    
    ArgumentCountAndMessageHandler handler = fHandler[key.buffer()];

    if (handler.first != static_cast<int>(handlerArgs.size())) {
        HIPHOP_LOG_STDERR_COLOR("Incorrect WebHostUI method argument count");
        return;
    }

    handler.second(handlerArgs);
}
