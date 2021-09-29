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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <dlfcn.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Path.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

static char* getImagePath(char* buf, int sz)
{
    memset(buf, 0, sz);
    Dl_info dl_info;

    if (dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) == 0) {
        HIPHOP_LOG_STDERR(dlerror());
        return buf;
    }

    strncpy(buf, dl_info.dli_fname, sz - 1);

    return buf;
}

static char* getExecutablePath(char* buf, int sz)
{
    memset(buf, 0, sz);
    ssize_t len = readlink("/proc/self/exe", buf, sz - 1);

    if (len == -1) {
        HIPHOP_LOG_STDERR_ERRNO("Could not determine executable path");
        return buf;
    }

    return buf;
}

String path::getBinaryPath()
{
    char imgPath[PATH_MAX];
    getImagePath(imgPath, sizeof(imgPath));
    char exePath[PATH_MAX];
    getExecutablePath(exePath, sizeof(exePath));
    const char *path = strcmp(imgPath, exePath) == 0 ? exePath : imgPath;
    return String(path);
}

String path::getLibraryPath()
{
    char imgPath[PATH_MAX];
    getImagePath(imgPath, sizeof(imgPath));
    char exePath[PATH_MAX];
    getExecutablePath(exePath, sizeof(exePath));

    if (strcmp(imgPath, exePath) != 0) {
        void* handle = dlopen(imgPath, RTLD_LAZY | RTLD_NOLOAD);
        void* addr;

        String path(dirname(imgPath)); 

        if (handle != 0) {
            addr = dlsym(handle, "lv2ui_descriptor");
            if (addr != 0) {
                dlclose(handle);
                return path + "/" + kDefaultLibrarySubdirectory; // LV2
            }

            addr = dlsym(handle, "lv2_descriptor");
            if (addr != 0) {
                dlclose(handle);
                return path + "/" + kDefaultLibrarySubdirectory; // LV2
            }

            addr = dlsym(handle, "main");
            if (addr != 0) {
                dlclose(handle);
                return path + "/" + kDefaultLibrarySubdirectory; // VST2
            }

            addr = dlsym(handle, "GetPluginFactory");
            if (addr != 0) {
                dlclose(handle);
                return path + "/../Resources"; // VST3
            }

            dlclose(handle);
        }
    }

    // Standalone
    return String(dirname(exePath)) + "/" + kDefaultLibrarySubdirectory;
}

String path::getCachesPath()
{
    String path;
    struct passwd *pw = getpwuid(getuid());
    path += pw->pw_dir;
    path += "/.config/" XSTR(PLUGIN_BIN_BASENAME);
    mkdir(path, 0777);
    path += "/" + kDefaultCacheSubdirectory;
    mkdir(path, 0777);
    return path;
}
