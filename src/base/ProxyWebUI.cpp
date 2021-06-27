/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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
{
    // Automatically scale up the plugin UI so its contents do not look small
    // on high pixel density displays, known as HiDPI or Retina.
    float k = platform::getSystemDisplayScaleFactor();
    fInitWidth = k * baseWidth;
    fInitHeight = k * baseHeight;
    setSize(fInitWidth, fInitHeight);
#ifdef DISTRHO_OS_WINDOWS
    setSize(fInitWidth, fInitHeight); // WINSIZEBUG: why repeating 2x?
    fWebWidget.setSize(fInitWidth, fInitHeight); // WINSIZEBUG: why?
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

void ProxyWebUI::handleWebWidgetContentLoadFinished()
{
    // no-op, just let derived classes now
    webContentReady();
}

void ProxyWebUI::handleWebWidgetScriptMessageReceived(const ScriptValueVector& args)
{
    if (args[0].getString() != "WebUI") {
        webMessageReceived(args); // passthrough
        return;
    }

    // It is not possible to implement synchronous calls without resorting
    // to dirty hacks, for such cases fulfill getter promises instead.

    String method = args[1].getString();
    int argc = args.size() - 2;

    // TODO: use a hashtable instead of comparing strings ?

    if (method == "flushInitMessageQueue") {
        flushInitMessageQueue();

    } else if (method == "getInitWidth") {
        webPostMessage({"WebUI", "getInitWidth", static_cast<double>(getInitWidth())});

    } else if (method == "getInitHeight") {
        webPostMessage({"WebUI", "getInitHeight", static_cast<double>(getInitHeight())});

    } else if (method == "getWidth") {
        webPostMessage({"WebUI", "getWidth", static_cast<double>(getWidth())});

    } else if (method == "getHeight") {
        webPostMessage({"WebUI", "getHeight", static_cast<double>(getHeight())});

    } else if ((method == "setWidth") && (argc == 1)) {
        setWidth(static_cast<uint>(args[2].getDouble()));

    } else if ((method == "setHeight") && (argc == 1)) {
        setHeight(static_cast<uint>(args[2].getDouble()));

    } else if (method == "isResizable") {
        webPostMessage({"WebUI", "isResizable", isResizable()});

    } else if ((method == "setSize") && (argc == 2)) {
        setSize(
            static_cast<uint>(args[2].getDouble()), // width
            static_cast<uint>(args[3].getDouble())  // height
        );

    } else if ((method == "editParameter") && (argc == 2)) {
        editParameter(
            static_cast<uint32_t>(args[2].getDouble()), // index
            static_cast<bool>(args[3].getBool())        // started
        );

    } else if ((method == "setParameterValue") && (argc == 2)) {
        setParameterValue(
            static_cast<uint32_t>(args[2].getDouble()), // index
            static_cast<float>(args[3].getDouble())     // value
        );

#if (DISTRHO_PLUGIN_WANT_STATE == 1)
    } else if ((method == "setState") && (argc == 2)) {
        setState(
            args[2].getString(), // key
            args[3].getString()  // value
        );
#endif // DISTRHO_PLUGIN_WANT_STATE == 1

    } else {
        DISTRHO_LOG_STDERR_COLOR("Invalid call to ProxyWebUI method");
    }
}

void ProxyWebUI::handleWebWidgetKeyboardEvent(void* arg0, void* arg1)
{
    platform::sendKeyboardEventToHost(arg0, arg1);
}
