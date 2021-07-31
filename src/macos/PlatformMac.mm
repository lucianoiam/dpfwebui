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

#import <AppKit/AppKit.h>

#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/syslimits.h>

#include "Platform.hpp"
#include "macro.h"


// MACSIZEBUG - Support for patched DistrhoUI.cpp
// It currently seems that on Mac+HiDPI the host is getting dimensions already
// multiplied by the screen scale factor, resulting for example in REAPER
// showing an excessively large canvas that does not tightly wrap the plugin UI,
// or Live showing a floating window with plenty of empty space. Likely the host
// is expecting unscaled values, ie. those specified by NSView frames. The issue
// does not affect the standalone version of the plugin. Jun '21.

namespace DISTRHO {
    float FIXME_MacScaleFactor() {
        return platform::getSystemDisplayScaleFactor();
    }
}


USE_NAMESPACE_DISTRHO

float platform::getSystemDisplayScaleFactor()
{
    return [NSScreen mainScreen].backingScaleFactor;
}

String platform::getBinaryPath()
{
    Dl_info dl_info;
    if (dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) != 0) {
        return String(dl_info.dli_fname);
    } else {
        HIPHOP_LOG_STDERR(dlerror());
        return String();
    }
}

String platform::getLibraryPath()
{
    // There is no DPF method for querying plugin format during runtime
    // Mac VST is the only special case though
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

String platform::getTemporaryPath()
{
    return String(); // not implemented
}
