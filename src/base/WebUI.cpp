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

#include "WebUI.hpp"
#include "Window.hpp"

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

WebUI::WebUI(uint width, uint height, uint32_t backgroundColor)
    : UI(width, height)
    , fWebView(*this)
    , fBackgroundColor(backgroundColor)
    , fDisplayed(false)
{
    // Expand size if needed based on system display scaling configuration
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * getWidth(), scaleFactor * getHeight());
    // Set web view background color as early as possible to reduce flicker
    fWebView.resize(getSize());
    fWebView.setBackgroundColor(fBackgroundColor);
    fWebView.reparent(getParentWindow().getWindowId());
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
    cairo_t* cr = getParentWindow().getGraphicsContext().cairo;
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
    String js = String(
#include "base/webui.js"
    );
    fWebView.injectScript(js);
    fWebView.navigate("file://" + platform::getResourcePath() + "/index.html");
    fWebView.start();
}

void WebUI::onResize(const ResizeEvent& ev)
{
    fWebView.resize(ev.size);
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    webViewPostMessage({"WebUI", "parameterChanged", index, value});
}

#if DISTRHO_PLUGIN_WANT_STATE

void WebUI::stateChanged(const char* key, const char* value)
{
    webViewPostMessage({"WebUI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE

bool WebUI::webViewScriptMessageReceived(const ScriptValueVector& args)
{
    if ((args.size() < 4) || (args[0].getString() != "WebUI")) {
        return false;
    }
    String method = args[1].getString();
    if (method == "editParameter") {
        editParameter(
            static_cast<uint32_t>(args[2].getDouble()), // index
            static_cast<bool>(args[3].getBool())        // started
        );
    } else if (method == "setParameterValue") {
        setParameterValue(
            static_cast<uint32_t>(args[2].getDouble()), // index
            static_cast<float>(args[3].getDouble())     // value
        );
#if DISTRHO_PLUGIN_WANT_STATE
    } else if (method == "setState") {
        setState(
            args[2].getString(), // key
            args[3].getString()  // value
        );
#endif
    } else {
        DISTRHO_LOG_STDERR_COLOR("Invalid call to native WebUI method");
    }
    return true;
}
