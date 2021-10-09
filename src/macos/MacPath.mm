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

#import <Foundation/Foundation.h>

#include <cstring>

#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#include "Path.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

String path::getBinaryPath()
{
    Dl_info dl_info;
    String path;

    // dladdr() works for executables and dynamic libraries

    if (dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) != 0) {
        path = dl_info.dli_fname;
    } else {
        HIPHOP_LOG_STDERR(dlerror());
    }

    return path;
}

String path::getLibraryPath()
{
    char binPath[PATH_MAX];
    strcpy(binPath, getBinaryPath());
    String path(dirname(binPath));

    // dlopen() only works for dynamic libraries
    void* handle = dlopen(binPath, RTLD_LAZY | RTLD_NOLOAD);
    void* addr;

    if (handle != 0) {
        addr = dlsym(handle, "lv2ui_descriptor");
        if (addr != 0) {
            dlclose(handle);
            return path + "/" + kBundleLibrarySubdirectory; // LV2
        }

        addr = dlsym(handle, "lv2_descriptor");
        if (addr != 0) {
            dlclose(handle);
            return path + "/" + kBundleLibrarySubdirectory; // LV2
        }

        addr = dlsym(handle, "VSTPluginMain");
        if (addr != 0) {
            dlclose(handle);
            return path + "/../Resources"; // VST2
        }

        addr = dlsym(handle, "GetPluginFactory");
        if (addr != 0) {
            dlclose(handle);
            return path + "/../Resources"; // VST3
        }

        dlclose(handle);
    }

    // Standalone
    return path + "/" + kNoBundleLibrarySubdirectory;
}

String path::getCachesPath()
{
    NSArray* p = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    String path = String([[p lastObject] cStringUsingEncoding:NSUTF8StringEncoding])
        + "/" XSTR(PLUGIN_BIN_BASENAME);
    
    mkdir(path, 0777);

    return path;
}
