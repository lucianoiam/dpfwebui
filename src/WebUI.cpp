/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
 *
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

UI* DISTRHO::createUI()
{
    return new WebUI;
}

WebUI::WebUI()
    : fWebView(*this)
    , fParentWindowId(0)
{
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * DISTRHO_UI_INITIAL_WIDTH, scaleFactor * DISTRHO_UI_INITIAL_HEIGHT);
#if defined(DISTRHO_UI_BACKGROUND_COLOR) && defined(DGL_OPENGL)
    glClearColor(UNPACK_RGBA(DISTRHO_UI_BACKGROUND_COLOR, GLfloat));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    fWebView.resize(getSize());
    fWebView.addScriptMessageHandler(String("dpf"));
    fWebView.injectScript(String("window._testString = 'Hello World';"));
    fWebView.navigate("file://" + platform::getResourcePath() + "/index.html");
}

void WebUI::onDisplay()
{
    const Window& window = getParentWindow();
#if defined(DISTRHO_UI_BACKGROUND_COLOR) && defined(DGL_CAIRO)
    cairo_t* cr = window.getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, UNPACK_RGBA(DISTRHO_UI_BACKGROUND_COLOR, double));
    cairo_paint(cr);
#endif
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = window.getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        fWebView.reparent(fParentWindowId);
    }
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    (void)index;
    (void)value;
}

void WebUI::onResize(const ResizeEvent& ev)
{
    fWebView.resize(ev.size);
}

void WebUI::handleWebViewScriptMessage(String name, const ScriptMessageArguments& args)
{
    if ((name != "dpf") || args.empty()) {
        return;
    }
    String method = args[0].asString();
    // TODO
    if (method == "editParameter") {
        // uint32_t index, bool started
    } else if (method == "setParameterValue") {
        // uint32_t index, float value
        ::printf("%s(%u,%.2f)\n", static_cast<const char*>(method),
            static_cast<uint32_t>(args[1].asDouble()), static_cast<float>(args[2].asDouble()));
#if DISTRHO_PLUGIN_WANT_STATE
    } else if (method == "setState") {
        // const char* key, const char* value
#endif
    }
}
