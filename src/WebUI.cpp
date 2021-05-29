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

#include "RuntimePath.hpp"

USE_NAMESPACE_DISTRHO

WebUI::WebUI()
    : UI(800, 600)  // TODO: avoid arbitrary size, plugin should be resizable
    , fParentWindowId(0)
{}

void WebUI::onDisplay()
{
    // Avoid glitches while the web view initializes
    clearBackground();
    
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = getParentWindow().getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        reparent(fParentWindowId);
    }
}

String WebUI::getContentUrl()
{
    return "file://" + rtpath::getResourcePath() + "/index.html";
}

void WebUI::clearBackground()
{
    uint rgba = getBackgroundColor();
    float r = (rgba >> 24) / 255.f;
    float g = ((rgba & 0x00ff0000) >> 16) / 255.f;
    float b = ((rgba & 0x0000ff00) >> 8) / 255.f;
    float a = (rgba & 0x000000ff) / 255.f;
#ifdef DGL_CAIRO
    cairo_t* cr = getParentWindow().getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_paint(cr);
#endif
#ifdef DGL_OPENGL
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}
