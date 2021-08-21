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

#include "Platform.hpp"
#include "macro.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

USE_NAMESPACE_DISTRHO

// Explanation for the GCC warnings https://github.com/chriskohlhoff/asio/issues/631
typedef HRESULT (WINAPI* PFN_GetProcessDpiAwareness)(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
typedef HRESULT (WINAPI* PFN_GetScaleFactorForMonitor)(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

float platform::getDisplayScaleFactor(uintptr_t window)
{
    float k = 1.f;
    const HMODULE hm = LoadLibrary("Shcore.dll");

    if (hm == 0) {
        return k;
    }

    const PFN_GetProcessDpiAwareness GetProcessDpiAwareness
        = (PFN_GetProcessDpiAwareness)GetProcAddress(hm, "GetProcessDpiAwareness");
    const PFN_GetScaleFactorForMonitor GetScaleFactorForMonitor
        = (PFN_GetScaleFactorForMonitor)GetProcAddress(hm, "GetScaleFactorForMonitor");

    PROCESS_DPI_AWARENESS dpiAware;

    if ((GetProcessDpiAwareness != 0) && (GetScaleFactorForMonitor != 0)
            && (SUCCEEDED(GetProcessDpiAwareness(0, &dpiAware)))
            && (dpiAware != PROCESS_DPI_UNAWARE)) {

        const HMONITOR hMon = MonitorFromWindow((HWND)window, MONITOR_DEFAULTTOPRIMARY);

        DEVICE_SCALE_FACTOR scaleFactor;

        if (SUCCEEDED(GetScaleFactorForMonitor(hMon, &scaleFactor))) {
            k = static_cast<float>(scaleFactor) / 100.f;
        }
    }

    FreeLibrary(hm);

    return k;
}

String platform::getBinaryPath()
{
    char path[MAX_PATH];
    
    if (GetModuleFileName((HINSTANCE)&__ImageBase, path, sizeof(path)) == 0) {
        HIPHOP_LOG_STDERR_INT("Could not determine module path", GetLastError());
        path[0] = '\0';
    }

    return String(path);
}

String platform::getLibraryPath()
{
    char path[MAX_PATH];
    strcpy(path, getBinaryPath());
    
    PathRemoveFileSpec(path);

    return String(path) + "\\" + kDefaultLibrarySubdirectory;
}

String platform::getTemporaryPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPF_Temp
    char tempPath[MAX_PATH];
    HRESULT result = SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_DEFAULT, tempPath);
    
    if (FAILED(result)) {
        HIPHOP_LOG_STDERR_INT("Could not determine user app data folder", result);
        return String();
    }

    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously due to permissions. C:\Users\< USERNAME >\AppData\Local\DPF_Temp\< HOST_BIN >
    char exePath[MAX_PATH];
    
    if (GetModuleFileName(0, exePath, sizeof(exePath)) == 0) {
        HIPHOP_LOG_STDERR_INT("Could not determine host executable path", GetLastError());
        return String();
    }

    LPSTR exeFilename = PathFindFileName(exePath);

    // The following call relies on a further Windows library called Pathcch, which is implemented
    // in api-ms-win-core-path-l1-1-0.dll and requires Windows 8.
    // Since the minimum plugin target is Windows 7 it is acceptable to use a deprecated function.
    //PathCchRemoveExtension(exeFilename, sizeof(exeFilename));
    PathRemoveExtension(exeFilename);
    strcat(tempPath, "\\DPF_Temp\\");
    strcat(tempPath, exeFilename);

    return String(tempPath);
}
