/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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

#include "Platform.hpp"

#include <cstring>
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <shellscalingapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shtypes.h>

#include "WinApiStub.hpp"
#include "log.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

USE_NAMESPACE_DISTRHO

String platform::getBinaryDirectoryPath()
{
    char path[MAX_PATH];
    ::strcpy(path, getBinaryPath());
    ::PathRemoveFileSpec(path);
    return String(path);
}

String platform::getBinaryPath()
{
    char path[MAX_PATH];
    if (::GetModuleFileName((HINSTANCE)&__ImageBase, path, sizeof(path)) == 0) {
        LOG_STDERR_INT("Could not determine module path", ::GetLastError());
        path[0] = '\0';
    }
    return String(path);
}

String platform::getSharedLibraryPath()
{
    return getBinaryPath();
}

String platform::getExecutablePath()
{
    return getBinaryPath();
}

String platform::getTemporaryPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPFTemp
    char tempPath[MAX_PATH];
    HRESULT result = ::SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_DEFAULT, tempPath);
    if (FAILED(result)) {
        LOG_STDERR_INT("Could not determine user app data folder", result);
        return String();
    }

    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously due to permissions. C:\Users\< USERNAME >\AppData\Local\DPFTemp\< HOST_BIN >
    char exePath[MAX_PATH];
    if (::GetModuleFileName(0, exePath, sizeof(exePath)) == 0) {
        LOG_STDERR_INT("Could not determine host executable path", ::GetLastError());
        return String();
    }

    LPSTR exeFilename = ::PathFindFileName(exePath);
    // The following call relies on a further Windows library called Pathcch, which is implemented
    // in in api-ms-win-core-path-l1-1-0.dll and requires Windows 8.
    // Since the minimum plugin target is Windows 7 it is acceptable to use a deprecated function.
    //::PathCchRemoveExtension(exeFilename, sizeof(exeFilename));
    ::PathRemoveExtension(exeFilename);

    ::strcat(tempPath, "\\DPFTemp\\");
    ::strcat(tempPath, exeFilename);
    
    return String(tempPath);
}

float platform::getSystemDisplayScaleFactor()
{
    float k = 1.f;
    PROCESS_DPI_AWARENESS dpiAware;
    if (SUCCEEDED(winstub::GetProcessDpiAwareness(0, &dpiAware))) {
        if (dpiAware != PROCESS_DPI_UNAWARE) {
            HMONITOR hMon = ::MonitorFromWindow(::GetConsoleWindow(), MONITOR_DEFAULTTOPRIMARY);
            DEVICE_SCALE_FACTOR scaleFactor;
            if (SUCCEEDED(winstub::GetScaleFactorForMonitor(hMon, &scaleFactor))) {
                if (scaleFactor != DEVICE_SCALE_FACTOR_INVALID) {
                    k = static_cast<float>(scaleFactor) / 100.f;
                }
            }
        } else {
            // Process is not DPI-aware, do not scale
        }
    }
    return k;
}