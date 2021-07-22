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

#include "WebHostUI.hpp"

#include "dgl/Application.hpp"

#include "Platform.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

// It currently seems that on Mac+HiDPI the host is getting dimensions already
// multiplied by the screen scale factor, resulting for example in REAPER
// showing an excessively large canvas that does not tightly wrap the plugin UI,
// or Live showing a floating window with plenty of empty space. Likely the host
// is expecting unscaled values, ie. those specified by NSView frames. The issue
// does not affect the standalone version of the plugin. Jun '21.
#ifdef DISTRHO_OS_MAC
namespace DISTRHO {
    // MACSIZEBUG - Support for patched DistrhoUI.cpp
    float FIXME_MacScaleFactor() {
        return platform::getSystemDisplayScaleFactor();
    }
}
#endif

WebHostUI::WebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : UI(baseWidth, baseHeight)
    , fWebWidget(this)
    , fFlushedInitMsgQueue(false)
    , fBackgroundColor(backgroundColor)
{
    // Platform functions can be called from anywhere. However DGL app object
    // is not a singleton and there is a special case in PlatformLinux.cpp that
    // makes it necessary to distinguish standalone vs. plugin during runtime.
    // Note that the web widget is already initialized at this point so this
    // function always returns false when called from web widget constructors. 
    platform::setRunningStandalone(getWindow().getApp().isStandalone());

    // Automatically scale up the plugin UI so its contents do not look small
    // on high pixel density displays, known as HiDPI or Retina.
    float k = platform::getSystemDisplayScaleFactor();
    fInitWidth = k * baseWidth;
    fInitHeight = k * baseHeight;
    setSize(fInitWidth, fInitHeight);

#ifdef DISTRHO_OS_WINDOWS
    // WINSIZEBUG: Why setSize() call needs to be repeated 2x?
    setSize(fInitWidth, fInitHeight);
#endif

    fWebWidget.setSize(fInitWidth, fInitHeight);
    fWebWidget.setBackgroundColor(fBackgroundColor);
    fWebWidget.setEventHandler(this);
#ifdef HIPHAP_PRINT_TRAFFIC
    fWebWidget.setPrintTraffic(true);
#endif

    String js = String(
#include "ui/distrho-ui.js.include"
    );
    fWebWidget.injectScript(js);

    String url = "file://" + platform::getLibraryPath() + "/ui/index.html";
    fWebWidget.navigate(url);
}

void WebHostUI::onDisplay()
{
#ifdef DGL_OPENGL
    // Clear background for OpenGL
    glClearColor(DISTRHO_UNPACK_RGBA_NORM(fBackgroundColor, GLfloat));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
#ifdef DGL_CAIRO
    // Clear background for Cairo
    cairo_t* cr = ((CairoGraphicsContext&)getWindow().getGraphicsContext()).handle;
    cairo_set_source_rgba(cr, DISTRHO_UNPACK_RGBA_NORM(fBackgroundColor, double));
    cairo_paint(cr);
#endif
}

void WebHostUI::uiReshape(uint width, uint height)
{
    fWebWidget.setSize(width, height);
}

void WebHostUI::parameterChanged(uint32_t index, float value)
{
    webPostMessage({"UI", "parameterChanged", index, value});
}

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

void WebHostUI::stateChanged(const char* key, const char* value)
{
    webPostMessage({"UI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

uint WebHostUI::getInitWidth() const
{
    return fInitWidth;
}

uint WebHostUI::getInitHeight() const
{
    return fInitHeight;
}

void WebHostUI::webPostMessage(const ScriptValueVector& args) {
    if (fFlushedInitMsgQueue) {
        fWebWidget.postMessage(args);
    } else {
        fInitMsgQueue.push_back(args);
    }
}

void WebHostUI::flushInitMessageQueue()
{
    if (fFlushedInitMsgQueue) {
        return;
    }

    fFlushedInitMsgQueue = true;

    for (InitMessageQueue::iterator it = fInitMsgQueue.begin(); it != fInitMsgQueue.end(); ++it) {
        fWebWidget.postMessage(*it);
    }
    
    fInitMsgQueue.clear();
}

void WebHostUI::setKeyboardFocus(bool focus)
{
    fWebWidget.setKeyboardFocus(focus);
}

void WebHostUI::handleWebWidgetContentLoadFinished()
{
    // no-op, just let derived classes now
    webContentReady();
}

#define kArg0 2
#define kArg1 3

void WebHostUI::handleWebWidgetScriptMessageReceived(const ScriptValueVector& args)
{
    if (args[0].getString() != "UI") {
        webMessageReceived(args); // passthrough
        return;
    }

    // It is not possible to implement JS synchronous calls that return values
    // without resorting to dirty hacks. Use JS async functions instead, and
    // fulfill their promises here. See for example isResizable() below.

    String method = args[1].getString();
    int argc = args.size() - kArg0;

    // How about using a hashtable of methods instead of comparing strings?

    if (method == "flushInitMessageQueue") {
        flushInitMessageQueue();

    } else if ((method == "setKeyboardFocus") && (argc == 1)) {
        setKeyboardFocus(static_cast<bool>(args[kArg0].getBool()));

    } else if (method == "getInitWidth") {
        webPostMessage({"UI", "getInitWidth", static_cast<double>(getInitWidth())});

    } else if (method == "getInitHeight") {
        webPostMessage({"UI", "getInitHeight", static_cast<double>(getInitHeight())});

    } else if (method == "getWidth") {
        webPostMessage({"UI", "getWidth", static_cast<double>(getWidth())});

    } else if (method == "getHeight") {
        webPostMessage({"UI", "getHeight", static_cast<double>(getHeight())});

    } else if ((method == "setWidth") && (argc == 1)) {
        setWidth(static_cast<uint>(args[kArg0].getDouble()));

    } else if ((method == "setHeight") && (argc == 1)) {
        setHeight(static_cast<uint>(args[kArg0].getDouble()));

    } else if (method == "isResizable") {
        webPostMessage({"UI", "isResizable", isResizable()});

    } else if ((method == "setSize") && (argc == 2)) {
        setSize(
            static_cast<uint>(args[kArg0].getDouble()), // width
            static_cast<uint>(args[kArg1].getDouble())  // height
        );
#ifdef DISTRHO_OS_WINDOWS
        // WINSIZEBUG: need to repeat setSize() call 2x
        setSize(
            static_cast<uint>(args[kArg0].getDouble()), // width
            static_cast<uint>(args[kArg1].getDouble())  // height
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

#if (DISTRHO_PLUGIN_WANT_STATE == 1)
    } else if ((method == "setState") && (argc == 2)) {
        setState(
            args[kArg0].getString(), // key
            args[kArg1].getString()  // value
        );
#endif // DISTRHO_PLUGIN_WANT_STATE == 1

    } else if (method == "isStandalone") {
        webPostMessage({"UI", "isStandalone", getWindow().getApp().isStandalone()});

    } else {
        HIPHAP_LOG_STDERR_COLOR("Invalid call to WebHostUI method");
    }
}
