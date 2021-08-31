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

USE_NAMESPACE_DISTRHO

LinuxWebHostUI::LinuxWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
{
    if (isEmbed()) {
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
    // TODO
}

uintptr_t LinuxWebHostUI::createStandaloneWindow()
{
    // TODO
    return 0;
}

void LinuxWebHostUI::processStandaloneEvents()
{
    // TODO
}
