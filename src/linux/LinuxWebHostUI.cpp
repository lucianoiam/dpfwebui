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

LinuxWebHostUI::LinuxWebHostUI(uint baseWidth, uint baseHeight,
        uint32_t backgroundColor, bool startLoading)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
{
    if (!shouldCreateWebView()) {
        return;
    }

    ChildProcessWebView* webview = new ChildProcessWebView();
    setWebView(webview); // base class owns web view

#ifdef LXWEBVIEW_GTK
    // Allow JavaScript code to detect the GTK webview and enable some
    // workarounds to compensate for the broken vw/vh/vmin/vmax CSS units and
    // non-working touch events for <input type="range"> elements.
    String js = String(
        "window.DISTRHO.quirks.brokenCSSViewportUnits = true;"
        "window.DISTRHO.quirks.brokenRangeInputTouch = true;"
    );
    webview->injectScript(js);
#endif // LXWEBVIEW_GTK

    if (startLoading) {
        load();
    }
}

LinuxWebHostUI::~LinuxWebHostUI()
{
    // TODO - standalone support
}

float LinuxWebHostUI::getDisplayScaleFactor(uintptr_t)
{
#ifdef LXWEBVIEW_GTK
    // WebKitGTK reads these environment variables automatically, however the
    // same scale value is needed for determining maximum view dimensions, see
    // get_display_scale_factor() in gtk_helper.c for more details.

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
#endif // LXWEBVIEW_GTK

#ifdef LXWEBVIEW_CEF
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
#endif // LXWEBVIEW_CEF

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
