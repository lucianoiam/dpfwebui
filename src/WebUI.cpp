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

#include "Platform.hpp"

USE_NAMESPACE_DISTRHO

WebUI::WebUI()
    : fParentWindowId(0)
{
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * DISTRHO_UI_INITIAL_WIDTH, scaleFactor * DISTRHO_UI_INITIAL_HEIGHT);
#ifdef DGL_OPENGL
    uint rgba = getBackgroundColor();
    GLfloat r = (rgba >> 24) / 255.f;
    GLfloat g = ((rgba & 0x00ff0000) >> 16) / 255.f;
    GLfloat b = ((rgba & 0x0000ff00) >> 8) / 255.f;
    GLfloat a = (rgba & 0x000000ff) / 255.f;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void WebUI::onDisplay()
{
    const Window& window = getParentWindow();
#ifdef DGL_CAIRO
    uint rgba = getBackgroundColor();
    double r = (rgba >> 24) / 255.f;
    double g = ((rgba & 0x00ff0000) >> 16) / 255.0;
    double b = ((rgba & 0x0000ff00) >> 8) / 255.0;
    double a = (rgba & 0x000000ff) / 255.0;
    cairo_t* cr = window.getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_paint(cr);
#endif
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = window.getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        reparent(fParentWindowId);
    }
}

String WebUI::getContentUrl()
{
    return "file://" + platform::getResourcePath() + "/index.html";
}
