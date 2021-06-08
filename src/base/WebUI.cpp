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

#define INIT_WIDTH  600
#define INIT_HEIGHT 300

// Matching <html> background color to INIT_BACKGROUND_RGBA greatly reduces
// flicker while the UI is being opened or resized.
#define INIT_BACKGROUND_RGBA 0xffffffff

USE_NAMESPACE_DISTRHO

WebUI::WebUI()
    : fWebView(*this)
    , fDisplayed(false)
{
    // Expand size if needed based on system display scaling configuration
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * INIT_WIDTH, scaleFactor * INIT_HEIGHT);
#ifdef DGL_OPENGL
    // Clear background for OpenGL
    glClearColor(DISTRHO_UNPACK_RGBA_NORM(INIT_BACKGROUND_RGBA, GLfloat));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    fWebView.setBackgroundColor(INIT_BACKGROUND_RGBA);
}

void WebUI::onDisplay()
{
#ifdef DGL_CAIRO
    // Clear background for Cairo
    cairo_t* cr = getParentWindow().getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, DISTRHO_UNPACK_RGBA_NORM(INIT_BACKGROUND_RGBA, double));
    cairo_paint(cr);
#endif
    // onDisplay() is meant for drawing and will be called multiple times
    if (fDisplayed) {
        return;
    }
    fDisplayed = true;
    // At this point UI initialization has settled down and it is safe to launch
    // resource intensive tasks like starting child processes or loading a URL.
    // It is also the appropriate place to trigger Edge's asynchronous init.
    // On Linux and Mac, BaseWebView::start() is a no-op. Loading web content
    // could be thought of as drawing the window and only needs to happen once,
    // actual drawing is handled by the web view. WebUI() constructor is not
    // the suitable place because it can be called successive times without the
    // window ever being displayed.
    fWebView.reparent(getParentWindow().getWindowId());
    fWebView.resize(getSize());
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
