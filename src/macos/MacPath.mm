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

    if (dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) != 0) {
        return String(dl_info.dli_fname);
    }

    HIPHOP_LOG_STDERR(dlerror());

    return String();
}

String path::getLibraryPath()
{
    getCachesPath();

    char path[PATH_MAX];
    strcpy(path, getBinaryPath());
    void *handle = dlopen(path, RTLD_NOLOAD);

    if (handle != 0) {
        void *addr = dlsym(handle, "VSTPluginMain");
        dlclose(handle);
        if (addr != 0) {
            return String(dirname(path)) + "/../Resources";
        }
    }

    strcpy(path, getBinaryPath().buffer());

    return String(dirname(path)) + "/" + kDefaultLibrarySubdirectory;
}

String path::getCachesPath()
{
    NSArray* p = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    String path = String([[p lastObject] cStringUsingEncoding:NSUTF8StringEncoding])
        + "/" XSTR(PLUGIN_BIN_BASENAME);
    mkdir(path, 0777);
    return path;
}
