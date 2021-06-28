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

#include "ProxyWebUI.hpp"

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

// It currently seems that on Mac+HiDPI the host is getting dimensions already
// multiplied by the screen scale factor, resulting for example in REAPER
// showing an excessively large canvas that does not tightly wrap the plugin UI,
// or Live showing a floating window with plenty of empty space. Likely the host
// is expecting unscaled values, ie. those specified by NSView frames. The issue
// does not affect the standalone version of the plugin. Jun '21.
#ifdef DISTRHO_OS_MAC
namespace DISTRHO {
    // Support for patched DistrhoUI.cpp
    float FIXME_MacScaleFactor() {
        return platform::getSystemDisplayScaleFactor();
    }
}
#endif

ProxyWebUI::ProxyWebUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : UI(baseWidth, baseHeight)
    , fWebWidget(getWindow())
    , fFlushedInitMsgQueue(false)
    , fBackgroundColor(backgroundColor)
    , fGrabKeyboardInput(false)
{
    // Automatically scale up the plugin UI so its contents do not look small
    // on high pixel density displays, known as HiDPI or Retina.
    float k = platform::getSystemDisplayScaleFactor();
    fInitWidth = k * baseWidth;
    fInitHeight = k * baseHeight;
    setSize(fInitWidth, fInitHeight);

#ifdef DISTRHO_OS_WINDOWS
    // WINSIZEBUG: these are noticeable only when system scale factor > 100%
    setSize(fInitWidth, fInitHeight); // why repeating 2x?
    fWebWidget.setSize(fInitWidth, fInitHeight); // why?
#endif

    fWebWidget.setBackgroundColor(fBackgroundColor);
    fWebWidget.setEventHandler(this);

    String js = String(
#include "base/webui.js"
    );
    fWebWidget.injectScript(js);

    String url = "file://" + platform::getResourcePath() + "/index.html";
    fWebWidget.navigate(url);
}

void ProxyWebUI::onDisplay()
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

void ProxyWebUI::parameterChanged(uint32_t index, float value)
{
    webPostMessage({"WebUI", "parameterChanged", index, value});
}

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

void ProxyWebUI::stateChanged(const char* key, const char* value)
{
    webPostMessage({"WebUI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

uint ProxyWebUI::getInitWidth() const
{
    return fInitWidth;
}

uint ProxyWebUI::getInitHeight() const
{
    return fInitHeight;
}

void ProxyWebUI::webPostMessage(const ScriptValueVector& args) {
    if (fFlushedInitMsgQueue) {
        fWebWidget.postMessage(args);
    } else {
        fInitMsgQueue.push_back(args);
    }
}

void ProxyWebUI::flushInitMessageQueue()
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

void ProxyWebUI::setGrabKeyboardInput(bool grab)
{
    // Host window will not receive keystroke events when grab is enabled
    fGrabKeyboardInput = grab;
}

void ProxyWebUI::handleWebWidgetContentLoadFinished()
{
    // no-op, just let derived classes now
    webContentReady();
}

#define kArg0 2
#define kArg1 3

void ProxyWebUI::handleWebWidgetScriptMessageReceived(const ScriptValueVector& args)
{
    if (args[0].getString() != "WebUI") {
        webMessageReceived(args); // passthrough
        return;
    }

    // It is not possible to implement synchronous calls without resorting
    // to dirty hacks, for such cases fulfill getter promises instead.

    String method = args[1].getString();
    int argc = args.size() - kArg0;

    // TODO: use a hashtable instead of comparing strings ?

    if (method == "flushInitMessageQueue") {
        flushInitMessageQueue();

    } else if ((method == "setGrabKeyboardInput") && (argc == 1)) {
        setGrabKeyboardInput(static_cast<bool>(args[kArg0].getBool()));

    } else if (method == "getInitWidth") {
        webPostMessage({"WebUI", "getInitWidth", static_cast<double>(getInitWidth())});

    } else if (method == "getInitHeight") {
        webPostMessage({"WebUI", "getInitHeight", static_cast<double>(getInitHeight())});

    } else if (method == "getWidth") {
        webPostMessage({"WebUI", "getWidth", static_cast<double>(getWidth())});

    } else if (method == "getHeight") {
        webPostMessage({"WebUI", "getHeight", static_cast<double>(getHeight())});

    } else if ((method == "setWidth") && (argc == 1)) {
        setWidth(static_cast<uint>(args[kArg0].getDouble()));

    } else if ((method == "setHeight") && (argc == 1)) {
        setHeight(static_cast<uint>(args[kArg0].getDouble()));

    } else if (method == "isResizable") {
        webPostMessage({"WebUI", "isResizable", isResizable()});

    } else if ((method == "setSize") && (argc == 2)) {
        setSize(
            static_cast<uint>(args[kArg0].getDouble()), // width
            static_cast<uint>(args[kArg1].getDouble())  // height
        );
#ifdef DISTRHO_OS_WINDOWS
        // WINSIZEBUG: repeat 2x
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

    } else {
        DISTRHO_LOG_STDERR_COLOR("Invalid call to ProxyWebUI method");
    }
}

void ProxyWebUI::handleWebWidgetKeyboardEvent(void* event)
{
    if (!fGrabKeyboardInput) {
        platform::sendKeyboardEventToHost(event);
    }
}
