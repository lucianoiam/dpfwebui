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
#include <X11/Xresource.h>

#include "LinuxWebHostUI.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

LinuxWebHostUI::LinuxWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
{
    if (shouldCreateWebView()) {
        setWebView(new ChildProcessWebView()); // base class owns web view
    }
}

LinuxWebHostUI::~LinuxWebHostUI()
{
    // TODO - standalone support
}

float LinuxWebHostUI::getDisplayScaleFactor(uintptr_t)
{
#if LXWEBVIEW_TYPE == gtk
    // WebKitGTK reads these environment variables automatically, however the
    // same scale value is needed for determining maximum view dimensions, see
    // get_display_scale_factor() in gtk_helper.c

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
#endif // LXWEBVIEW_TYPE == gtk

#if LXWEBVIEW_TYPE == cef
    // CEF helper also reads Xft.dpi, see CefHelper::getZoomLevel()
    
    ::Display* const display = XOpenDisplay(nullptr);

    XrmInitialize();

    if (char* const rms = XResourceManagerString(display)) {
        if (const XrmDatabase sdb = XrmGetStringDatabase(rms)) {
            char* type = nullptr;
            XrmValue ret;

            if (XrmGetResource(sdb, "Xft.dpi", "String", &type, &ret)
                    && (ret.addr != nullptr) && (type != nullptr)
                    && (std::strncmp("String", type, 6) == 0)) {
                if (const float dpi = std::atof(ret.addr)) {
                    XCloseDisplay(display);
                    return dpi / 96.f;
                }
            }
        }
    }

    XCloseDisplay(display);
#endif // LXWEBVIEW_TYPE == cef

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
