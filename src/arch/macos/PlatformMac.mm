/*
 * dpf-webui
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

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

String platform::getBinaryDirectoryPath()
{
    char path[PATH_MAX];
    ::strcpy(path, getBinaryPath());
    return String(::dirname(path));
}

String platform::getBinaryPath()
{
    Dl_info dl_info;
    if (::dladdr((void *)&__PRETTY_FUNCTION__, &dl_info) == 0) {
        DISTRHO_LOG_STDERR(::dlerror());
        return String();
    } else {
        return String(dl_info.dli_fname);
    }
}

String platform::getSharedLibraryPath()
{
    return getBinaryPath();
}

String platform::getExecutablePath()
{
    return getBinaryPath();
}

String platform::getResourcePath()
{
    // There is no DPF method for querying plugin format during runtime
    // Anyways the ideal solution is to modify the Makefile and rely on macros
    // Mac VST is the only special case
    char path[PATH_MAX];
    ::strcpy(path, getSharedLibraryPath());
    void *handle = ::dlopen(path, RTLD_NOLOAD);
    if (handle != 0) {
        void *addr = ::dlsym(handle, "VSTPluginMain");
        ::dlclose(handle);
        if (addr != 0) {
            return String(::dirname(path)) + "/../Resources";
        }
    }
    return getBinaryDirectoryPath() + "/" + kDefaultResourcesSubdirectory;
}

String platform::getTemporaryPath()
{
    return String(); // not implemented
}

float platform::getSystemDisplayScaleFactor()
{
    return [NSScreen mainScreen].backingScaleFactor;
}

void platform::sendKeyboardEventToHost(void* event)
{
    NSEvent *nsEvent = (NSEvent *)event;
    NSResponder *responder = [NSApplication sharedApplication].mainWindow.firstResponder;

    if (nsEvent.type == NSEventTypeKeyDown) {
        [responder keyDown:nsEvent];
    } else if (nsEvent.type == NSEventTypeKeyUp) {
        [responder keyUp:nsEvent];
    }
}
