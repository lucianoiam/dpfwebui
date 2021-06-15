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

#include "WebUI.hpp"
#include "Window.hpp"

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

// TODO - why the special case?
#ifdef DISTRHO_OS_WINDOWS
#define INIT_SCALE_FACTOR platform::getSystemDisplayScaleFactor()
#else
#define INIT_SCALE_FACTOR 1.f
#endif

WebUI::WebUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : UI(INIT_SCALE_FACTOR * baseWidth, INIT_SCALE_FACTOR * baseHeight)
    , fWebView(*this)
    , fBackgroundColor(backgroundColor)
    , fDisplayed(false)
    , fInitContentReady(false)
{
    // Automatically scale up the webview so its contents do not look small
    // on high pixel density displays (HiDPI / Retina)
    float k = platform::getSystemDisplayScaleFactor();
    uint width = k * baseWidth;
    uint height = k * baseHeight;
    setGeometryConstraints(width, height, true);
    setSize(width, height);
    fWebView.resize(getSize());
    fWebView.setBackgroundColor(fBackgroundColor);
    fWebView.reparent(getWindow().getNativeWindowHandle());
    String js = String(
#include "base/webui.js"
    );
    fWebView.injectScript(js);
}

void WebUI::onDisplay()
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
    // onDisplay() is meant for drawing and will be called multiple times
    if (fDisplayed) {
        return;
    }
    fDisplayed = true;
    // At this point UI initialization has settled down and it is time to launch
    // resource intensive tasks like loading a URL. It is also the appropriate
    // place for triggering Edge's asynchronous init. On Linux and Mac, method
    // BaseWebView::start() is a no-op. Loading web content could be thought of
    // as drawing the window and only needs to happen once, real drawing is
    // handled by the web views. WebUI() constructor is not a suitable place
    // for calling BaseWebView::navigate() because ctor/dtor can be called
    // successive times without the window ever being displayed (e.g. on Carla)
    String url = "file://" + platform::getResourcePath() + "/index.html";
    fWebView.navigate(url);
    fWebView.start();
}

void WebUI::onResize(const ResizeEvent& ev)
{
    fWebView.resize(ev.size);
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    webPostMessage({"WebUI", "parameterChanged", index, value});
}

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

void WebUI::stateChanged(const char* key, const char* value)
{
    webPostMessage({"WebUI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

void WebUI::webPostMessage(const ScriptValueVector& args) {
    if (fInitContentReady) {
        fWebView.postMessage(args);
    } else {
        fInitMsgQueue.push_back(args);
    }
}

void WebUI::flushInitMessageQueue()
{
    for (InitMessageQueue::iterator it = fInitMsgQueue.begin(); it != fInitMsgQueue.end(); ++it) {
        fWebView.postMessage(*it);
    }
    fInitMsgQueue.clear();
}

void WebUI::handleWebViewLoadFinished()
{
    fInitContentReady = true;
    webContentReady();
}

void WebUI::handleWebViewScriptMessageReceived(const ScriptValueVector& args)
{
    if (args[0].getString() != "WebUI") {
        webMessageReceived(args); // passthrough
        return;
    }
    String method = args[1].getString();
    int argc = args.size() - 2;
    if (method == "flushInitMessageQueue") {
        flushInitMessageQueue();
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
        DISTRHO_LOG_STDERR_COLOR("Invalid call to native WebUI method");
    }
}
