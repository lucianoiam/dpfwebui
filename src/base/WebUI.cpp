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

#include <deque>

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

void WebUI::webViewLoadFinished()
{
    // TODO - send state?

    // TEST CALL
    parameterChanged(123, 4.5);
}

bool WebUI::webViewScriptMessageReceived(const ScriptValueVector& args)
{
    if ((args.size() < 4) || (args[0].getString() != "WebUI")) {
        return false;
    }
    String method = args[1].getString();
    if (method == "editParameter") {
        uint32_t index = static_cast<uint32_t>(args[2].getDouble());
        bool started = static_cast<bool>(args[3].getBool());
        editParameter(index, started);
        ::printf("C++ received editParameter(%d,%s)\n", index, started ? "true" : "false");
    } else if (method == "setParameterValue") {
        // uint32_t index, float value
#if DISTRHO_PLUGIN_WANT_STATE
    } else if (method == "setState") {
        // const char* key, const char* value
#endif
    } else {
        DISTRHO_LOG_STDERR_COLOR("Invalid call to native WebUI method");
    }
    return true;
}
