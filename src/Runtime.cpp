/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
 *
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "Runtime.hpp"
#include "log.h"

USE_NAMESPACE_DISTRHO

#if defined(DISTRHO_OS_LINUX) || defined(DISTRHO_OS_MAC)

#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#ifdef DISTRHO_OS_LINUX
#include <linux/limits.h>
#elif DISTRHO_OS_MAC
#include <sys/syslimits.h>
#endif
#include <iostream>

char _dummy; // for dladdr()

String runtime::getBinaryDirectoryPath()
{
    char path[PATH_MAX];
    ::strcpy(path, getBinaryPath());
    return String(dirname(path));
}

String runtime::getBinaryPath()
{
    // DISTRHO_PLUGIN_TARGET_* macros are not available here
    // Is there a better way to differentiate we are being called from library or executable?
    String libPath = getSharedLibraryPath();     // path never empty even if running standalone
    void *handle = ::dlopen(libPath, RTLD_LAZY); // returns non-null on macOS also for standalone
    if (handle) {
        ::dlclose(handle);
        std::cout << libPath << std::endl;
        return libPath;
    }
    return getExecutablePath();
}

String runtime::getSharedLibraryPath()
{
    Dl_info dl_info;
    if (::dladdr((void *)&_dummy, &dl_info) == 0) {
        LOG_STDERR(::dlerror());
        return String();
    }
    return String(dl_info.dli_fname);
}

#ifdef DISTRHO_OS_LINUX

String runtime::getExecutablePath()
{
    char path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        LOG_STDERR_ERRNO("Could not read executable path");
        return String();
    }
    return String(path);
}

#elif DISTRHO_OS_MAC

String runtime::getExecutablePath()
{
    return getSharedLibraryPath();  // does the trick on macOS
}

#endif

#endif // DISTRHO_OS_LINUX || DISTRHO_OS_MAC



/*
String EdgeWebViewUI::getSharedLibraryDirectoryPath()
{
    WCHAR dllPath[MAX_PATH];

    if (GetModuleFileName((HINSTANCE)&__ImageBase, dllPath, sizeof(dllPath)) == 0) {
        LOG_STDERR_INT("Failed GetModuleFileName() call", GetLastError());
        return String();
    }

    PathRemoveFileSpec(dllPath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string path = converter.to_bytes(dllPath); 
    std::replace(path.begin(), path.end(), '\\', '/');

    return String(path.c_str());
}
*/
