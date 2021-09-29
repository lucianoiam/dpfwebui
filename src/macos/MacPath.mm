/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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
            return path + "/" + kDefaultLibrarySubdirectory; // LV2
        }

        addr = dlsym(handle, "lv2_descriptor");
        if (addr != 0) {
            dlclose(handle);
            return path + "/" + kDefaultLibrarySubdirectory; // LV2
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
    return path + "/" + kDefaultLibrarySubdirectory;
}

String path::getCachesPath()
{
    NSArray* p = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    String path = String([[p lastObject] cStringUsingEncoding:NSUTF8StringEncoding])
        + "/" XSTR(PLUGIN_BIN_BASENAME);
    
    mkdir(path, 0777);

    return path;
}
