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

#include "WebUI.hpp"
#include "Window.hpp"

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

WebUI::WebUI(uint width, uint height, uint32_t backgroundColor)
    : UI(platform::getSystemDisplayScaleFactor() * width,
         platform::getSystemDisplayScaleFactor() * height)
    , fWebView(*this)
    , fBackgroundColor(backgroundColor)
    , fDisplayed(false)
{
    fWebView.resize(getSize());
    fWebView.setBackgroundColor(fBackgroundColor);
    fWebView.reparent(getWindow().getNativeWindowHandle());
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
    webPostMessage({"WebUI", "parameterChanged", index, value});
}

#if DISTRHO_PLUGIN_WANT_STATE

void WebUI::stateChanged(const char* key, const char* value)
{
    webPostMessage({"WebUI", "stateChanged", key, value});
}

#endif // DISTRHO_PLUGIN_WANT_STATE

void WebUI::handleWebViewLoadFinished()
{
    // Currently no-op; let derived classes know about the event
    webContentReady();
}

void WebUI::handleWebViewScriptMessageReceived(const ScriptValueVector& args)
{
    if ((args.size() < 4) || (args[0].getString() != "WebUI")) {
        webMessageReceived(args); // passthrough
        return;
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
#endif // DISTRHO_PLUGIN_WANT_STATE
    } else {
        DISTRHO_LOG_STDERR_COLOR("Invalid call to native WebUI method");
    }
}
