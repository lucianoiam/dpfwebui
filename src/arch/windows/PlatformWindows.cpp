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
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <shellscalingapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shtypes.h>

#include "base/Platform.hpp"
#include "base/macro.h"

#include "extra/WinApiStub.hpp"

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
        DISTRHO_LOG_STDERR_INT("Could not determine module path", ::GetLastError());
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

String platform::getResourcePath()
{
    return getBinaryDirectoryPath().replace('\\', '/') + RESOURCES_SUBDIR_STRING;
}

String platform::getTemporaryPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPF_Temp
    char tempPath[MAX_PATH];
    HRESULT result = ::SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_DEFAULT, tempPath);
    if (FAILED(result)) {
        DISTRHO_LOG_STDERR_INT("Could not determine user app data folder", result);
        return String();
    }
    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously due to permissions. C:\Users\< USERNAME >\AppData\Local\DPF_Temp\< HOST_BIN >
    char exePath[MAX_PATH];
    if (::GetModuleFileName(0, exePath, sizeof(exePath)) == 0) {
        DISTRHO_LOG_STDERR_INT("Could not determine host executable path", ::GetLastError());
        return String();
    }
    LPSTR exeFilename = ::PathFindFileName(exePath);
    // The following call relies on a further Windows library called Pathcch, which is implemented
    // in in api-ms-win-core-path-l1-1-0.dll and requires Windows 8.
    // Since the minimum plugin target is Windows 7 it is acceptable to use a deprecated function.
    //::PathCchRemoveExtension(exeFilename, sizeof(exeFilename));
    ::PathRemoveExtension(exeFilename);
    ::strcat(tempPath, "\\DPF_Temp\\");
    ::strcat(tempPath, exeFilename);
    return String(tempPath);
}

float platform::getSystemDisplayScaleFactor()
{
    float k = 1.f;
    PROCESS_DPI_AWARENESS dpiAware;
    if (SUCCEEDED(stub::GetProcessDpiAwareness(0, &dpiAware))) {
        if (dpiAware != PROCESS_DPI_UNAWARE) {
            HMONITOR hMon = ::MonitorFromWindow(::GetConsoleWindow(), MONITOR_DEFAULTTOPRIMARY);
            DEVICE_SCALE_FACTOR scaleFactor;
            if (SUCCEEDED(stub::GetScaleFactorForMonitor(hMon, &scaleFactor))) {
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
