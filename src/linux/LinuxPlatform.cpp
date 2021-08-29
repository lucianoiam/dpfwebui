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

#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <linux/limits.h>

#include "Platform.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

float platform::getDisplayScaleFactor(uintptr_t)
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

void platform::openSystemWebBrowser(String& url)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "xdg-open %s", url.buffer());

    if (system(buf) != 0) {
        HIPHOP_LOG_STDERR_ERRNO("Could not open system web browser");
    }
}

static String getSharedLibraryPath()
{
    Dl_info dl_info;

    if (dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) == 0) {
        HIPHOP_LOG_STDERR(dlerror());
        return String();
    }
    
    return String(dl_info.dli_fname);
}

static String getExecutablePath()
{
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not determine executable path");
        return String();
    }

    return String(path);
}

String platform::getBinaryPath()
{
    if (isRunningStandalone()) {
        return getExecutablePath();
    } else {
        return getSharedLibraryPath();
    }
}

String platform::getLibraryPath()
{
    char path[PATH_MAX];
    strcpy(path, getBinaryPath());
    return String(dirname(path)) + "/" + kDefaultLibrarySubdirectory;
}

String platform::getTemporaryPath()
{
    return String(); // not implemented
}
