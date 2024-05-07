/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include "scaling.h"

float device_pixel_ratio()
{
    // Simulate Chromium device pixel ratio https://wiki.debian.org/MonitorDPI
    // Chromium will use the ratio between Xft/DPI (as set through XSETTINGS)
    // and the DPI reported by the X server (through xdpyinfo) as a scaling
    // factor to be used. GTK scale factor is also taken in account by Chromium.

    return xdpi_scale() * gdk_scale() * gdk_dpi_scale();
}

float xdpi_scale()
{
    return xft_dpi() / xdisplay_dpi();
}

float xft_dpi()
{
    Display* display = XOpenDisplay(NULL);
    XrmInitialize();

    char* rms = XResourceManagerString(display);
    float dpi = 96.f;

    if (rms != NULL) {
        XrmDatabase sdb = XrmGetStringDatabase(rms);

        if (sdb != NULL) {
            char* type = NULL;
            XrmValue ret;

            if (XrmGetResource(sdb, "Xft.dpi", "String", &type, &ret)
                    && (ret.addr != NULL) && (type != NULL)
                    && (strncmp("String", type, 6) == 0)) {
                float xftDpi = (float)atof(ret.addr);

                if (xftDpi > 0) {
                    dpi = xftDpi;
                }
            }
        }
    }

    XCloseDisplay(display);

    return dpi;
}

float xdisplay_dpi()
{
    Display* display = XOpenDisplay(NULL);
    return ((float)(DisplayWidth(display, 0)) * 25.4f /*mm to inch*/)
         / ((float)(DisplayWidthMM(display, 0)));
}

int gdk_scale()
{
    const char* s = getenv("GDK_SCALE");

    if (s != NULL) {
        int k = atoi(s);

        if (k > 0) {
            return k;
        }
    }

    return 1;
}

float gdk_dpi_scale()
{
    const char* s = getenv("GDK_DPI_SCALE");

    if (s != NULL) {
        float k = (float)atof(s);

        if (k > 0) {
            return k;
        }
    }

    return 1.f;
}
