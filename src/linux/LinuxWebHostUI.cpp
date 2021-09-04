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

#include <X11/Xlib.h>

#include "LinuxWebHostUI.hpp"
#include "LinuxPath.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

LinuxWebHostUI::LinuxWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
{
    const bool standalone = isStandalone();

    path::setRunningStandalone(standalone);

    if (!standalone) {
        ::Window parent = getParentWindowHandle();

        if (parent != 0) {
            Display* display = XOpenDisplay(0);

            if (display != 0) {
                XSetWindowBackground(display, parent, backgroundColor >> 8);
                XClearWindow(display, parent);
                XCloseDisplay(display);
            }
        }
    }

    // Special case for Linux: set background color before setting parent window
    // handle to prevent further flicker on startup, see helper.c create_view()

    fWebView.setBackgroundColor(getBackgroundColor());

    initWebView(fWebView);
}

LinuxWebHostUI::~LinuxWebHostUI()
{
    // TODO - standalone support
}

float LinuxWebHostUI::getDisplayScaleFactor(uintptr_t)
{
    // In the lack of a standard Linux interface for getting the display scale
    // factor, read GTK environment variables since the web view is GTK-based.

    const char* dpi;
    float k;

    dpi = getenv("GDK_SCALE");

    if ((dpi != 0) && (sscanf(dpi, "%f", &k) == 1)) {
        return k;
    }

    dpi = getenv("GDK_DPI_SCALE");

    if ((dpi != 0) && (sscanf(dpi, "%f", &k) == 1)) {
        return k;
    }

    return 1.f;
}

void LinuxWebHostUI::openSystemWebBrowser(String& url)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "xdg-open %s", url.buffer());

    if (system(buf) != 0) {
        HIPHOP_LOG_STDERR_ERRNO("Could not open system web browser");
    }
}

uintptr_t LinuxWebHostUI::createStandaloneWindow()
{
    // TODO - standalone support
    return 0;
}

void LinuxWebHostUI::processStandaloneEvents()
{
    // TODO - standalone support
}
