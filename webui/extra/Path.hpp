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

#ifndef PATH_HPP
#define PATH_HPP

#include <cstring>
#if DISTRHO_OS_LINUX
# include <dlfcn.h>
# include <libgen.h>
# include <pwd.h>
# include <unistd.h>
# include <linux/limits.h>
# include <sys/stat.h>
#endif
#if DISTRHO_OS_MAC
# include <dlfcn.h>
# include <libgen.h>
# include <sysdir.h>
# include <wordexp.h>
# include <sys/stat.h>
#endif
#if DISTRHO_OS_WINDOWS
# include <errhandlingapi.h>
# include <shlobj.h>
# include <shlwapi.h>
# include <shtypes.h>
#endif

// This header can be also included from non-plugin code (ie. Linux CEF helper)
#if ! defined(DPF_WEBUI_SKIP_DPF) && ! DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
# define USE_DPF
# include "DistrhoPluginUtils.hpp"
#endif

#include "distrho/extra/String.hpp"

#include "extra/macro.h"

#if DISTRHO_OS_WINDOWS
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

START_NAMESPACE_DISTRHO

enum class PluginFormat { Jack, LV2, CLAP, VST3, VST2, Unknown };

namespace PathSubdirectory {

    const String bundleLibrary = String("lib");
    const String nonBundleLibrary = String(XSTR(DPF_WEBUI_PLUGIN_BIN_BASENAME) "-lib");
    const String cache = String("cache");

}

struct Path
{
    static String getPluginBinary() noexcept
    {
#ifdef USE_DPF
        return String(getBinaryFilename()); // DistrhoUtils.cpp
#else
# ifdef DISTRHO_OS_WINDOWS
        CHAR buf[MAX_PATH];
        buf[0] = '\0';
        GetModuleFileNameA((HINSTANCE)&__ImageBase, buf, sizeof(buf));
        return String(buf);
# else
        // dladdr() works for executables and dynamic libs on Linux and macOS
        Dl_info info;
        dladdr((void *)&__PRETTY_FUNCTION__, &info);
        char buf[PATH_MAX];
        return String(realpath(info.dli_fname, buf));
# endif
#endif // USE_DPF
    }

    static String getPluginLibrary() noexcept
    {
        String path = getPluginBinary();
        path.truncate(path.rfind(DISTRHO_OS_SEP));

        switch (getPluginFormat()) {
            case PluginFormat::LV2: // LV2 bundles are not regular macOS bundles
                return path + DISTRHO_OS_SEP_STR + PathSubdirectory::bundleLibrary;
            case PluginFormat::CLAP:
            case PluginFormat::VST2:
#if defined(DISTRHO_OS_LINUX) || defined(DISTRHO_OS_WINDOWS)
                return path + DISTRHO_OS_SEP_STR + PathSubdirectory::nonBundleLibrary;
#endif
            case PluginFormat::VST3:
                return path.truncate(path.rfind(DISTRHO_OS_SEP)) + DISTRHO_OS_SEP_STR + "Resources";
            default:
                break;
        }

        return path + DISTRHO_OS_SEP_STR + PathSubdirectory::nonBundleLibrary;
    }

    static PluginFormat getPluginFormat() noexcept
    {
        PluginFormat format = PluginFormat::Unknown;

#ifdef USE_DPF
        const char* pfn = getPluginFormatName();

        if (strcmp(pfn, "JACK/Standalone") == 0) {
            format = PluginFormat::Jack;
        } else if (strcmp(pfn, "LV2") == 0) {
            format = PluginFormat::LV2;
        } else if (strcmp(pfn, "CLAP") == 0) {
            format = PluginFormat::CLAP;
        } else if (strcmp(pfn, "VST3") == 0) {
            format = PluginFormat::VST3;
        } else if (strcmp(pfn, "VST2") == 0) {
            format = PluginFormat::VST2;
        } else {
            format = PluginFormat::Unknown;
        }
#else
# if DISTRHO_OS_LINUX
        char exePath[PATH_MAX];
        exePath[0] = '\0';
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len == -1) {
            d_stderr2("Could not determine executable path");
            return format;
        }

        String binPath = getPluginBinary();

        if (strcmp(binPath, exePath) == 0) {
            format = PluginFormat::Jack;
        } else {
            void* handle = dlopen(binPath, RTLD_LAZY | RTLD_NOLOAD);

            if (handle != 0) {
                if ((dlsym(handle, "lv2_descriptor") != 0) || (dlsym(handle, "lv2ui_descriptor") != 0)) {
                    format = PluginFormat::LV2;
                } else if (dlsym(handle, "clap_entry") != 0) {
                    format = PluginFormat::CLAP;
                } else if (dlsym(handle, "GetPluginFactory") != 0) {
                    format = PluginFormat::VST3;
                } else if (dlsym(handle, "main") != 0) {
                    format = PluginFormat::VST2;
                }

                dlclose(handle);
            }
        }
# elif DISTRHO_OS_MAC
        void* handle = dlopen(getPluginBinary(), RTLD_LAZY | RTLD_NOLOAD);

        // dlopen() returns 0 for the standalone executable on macOS
        if (handle == 0) {
            format = PluginFormat::Jack;
        } else {
            if ((dlsym(handle, "lv2_descriptor") != 0) || (dlsym(handle, "lv2ui_descriptor") != 0)) {
                format = PluginFormat::LV2;
            } else if (dlsym(handle, "clap_entry") != 0) {
                format = PluginFormat::CLAP;
            } else if (dlsym(handle, "GetPluginFactory") != 0) {
                format = PluginFormat::VST3;
            } else if (dlsym(handle, "VSTPluginMain") != 0) {
                format = PluginFormat::VST2;
            }

            dlclose(handle);
        }
# elif DISTRHO_OS_WINDOWS
        // HISTANCE and HMODULE are interchangeable
        // https://stackoverflow.com/questions/2126657/how-can-i-get-hinstance-from-a-dll
        HMODULE hm = (HMODULE)&__ImageBase;

        if ((GetProcAddress(hm, "lv2_descriptor") != 0) || (GetProcAddress(hm, "lv2ui_descriptor") != 0)) {
            format = PluginFormat::LV2;
        } else if (GetProcAddress(hm, "clap_entry") != 0) {
            format = PluginFormat::CLAP;
        } else if (GetProcAddress(hm, "GetPluginFactory") != 0) {
            format = PluginFormat::VST3;
        } else if (GetProcAddress(hm, "VSTPluginMain") != 0) {
            format = PluginFormat::VST2;
        } else {
            format = PluginFormat::Jack;
        }
# endif
#endif // USE_DPF

        return format;
    }

    static String getUserData() noexcept
    {
#if DISTRHO_OS_LINUX
        String path;
        struct passwd *pw = getpwuid(getuid());
        path += pw->pw_dir;
        path += "/.config/" XSTR(DPF_WEBUI_PLUGIN_BIN_BASENAME);
        mkdir(path, 0777);
        path += "/" + PathSubdirectory::cache;
        mkdir(path, 0777);

        return path;
#elif DISTRHO_OS_MAC
        // Getting system directories without calling Objective-C
        // https://zameermanji.com/blog/2021/7/7/getting-standard-macos-directories/
        // https://www.manpagez.com/man/3/sysdir/
        char cachesPath[PATH_MAX];

        sysdir_search_path_enumeration_state state =
            sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_CACHES, SYSDIR_DOMAIN_MASK_USER);
        state = sysdir_get_next_search_path_enumeration(state, cachesPath);
        if (state == 0) {
            d_stderr2("Could not determine user caches directory");
            return String();
        }

        wordexp_t exp_result;
        wordexp(cachesPath, &exp_result, 0); // tilde expansion
        String path = String(exp_result.we_wordv[0]) + "/" XSTR(DPF_WEBUI_PLUGIN_BIN_BASENAME);
        wordfree(&exp_result);

        mkdir(path, 0777);

        return path;
#elif DISTRHO_OS_WINDOWS
        // Get path inside user files folder: C:\Users\< USERNAME >\AppData\Local\PluginName\cache
        char dataPath[MAX_PATH];
        const HRESULT result = SHGetFolderPathA(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_DEFAULT, dataPath);
        
        if (FAILED(result)) {
            d_stderr2("Could not determine user app data folder - %x", result);
            return String();
        }

        String path = String(dataPath) + "\\" XSTR(DPF_WEBUI_PLUGIN_BIN_BASENAME) "\\" + PathSubdirectory::cache;

        // Append host executable name to the cache path otherwise WebView2 controller initialization
        // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
        // simultaneously due to permissions. C:\Users\< USERNAME >\AppData\Local\PluginName\cache\< HOST_BIN >
        char exePath[MAX_PATH];
        
        if (GetModuleFileNameA(0, exePath, sizeof(exePath)) == 0) {
            d_stderr2("Could not determine host executable path - %x", GetLastError());
            return String();
        }

        LPSTR exeFilename = PathFindFileNameA(exePath);

        // The following call relies on a further Windows library called Pathcch, which is implemented
        // in api-ms-win-core-path-l1-1-0.dll and requires Windows 8.
        // Since the minimum plugin target is Windows 7 it is acceptable to use a deprecated function.
        //PathCchRemoveExtension(exeFilename, sizeof(exeFilename));
        PathRemoveExtensionA(exeFilename);
        path += "\\";
        path += exeFilename;

        return path;
#endif
    }

}; // struct path 

END_NAMESPACE_DISTRHO

#endif  // PATH_HPP
