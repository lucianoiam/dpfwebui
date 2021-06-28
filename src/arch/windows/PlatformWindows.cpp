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

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

String platform::getBinaryDirectoryPath()
{
    char path[MAX_PATH];
    strcpy(path, getBinaryPath());
    PathRemoveFileSpec(path);
    return String(path);
}

String platform::getBinaryPath()
{
    char path[MAX_PATH];
    if (GetModuleFileName((HINSTANCE)&__ImageBase, path, sizeof(path)) == 0) {
        DISTRHO_LOG_STDERR_INT("Could not determine module path", GetLastError());
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
    return getBinaryDirectoryPath() + "\\" + kDefaultResourcesSubdirectory;
}

String platform::getTemporaryPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPF_Temp
    char tempPath[MAX_PATH];
    HRESULT result = SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_DEFAULT, tempPath);
    if (FAILED(result)) {
        DISTRHO_LOG_STDERR_INT("Could not determine user app data folder", result);
        return String();
    }

    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously due to permissions. C:\Users\< USERNAME >\AppData\Local\DPF_Temp\< HOST_BIN >
    char exePath[MAX_PATH];
    if (GetModuleFileName(0, exePath, sizeof(exePath)) == 0) {
        DISTRHO_LOG_STDERR_INT("Could not determine host executable path", GetLastError());
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

float platform::getSystemDisplayScaleFactor()
{
    float k = 1.f;
    PROCESS_DPI_AWARENESS dpiAware;

    if (SUCCEEDED(stub::GetProcessDpiAwareness(0, &dpiAware))) {
        if (dpiAware != PROCESS_DPI_UNAWARE) {
            HMONITOR hMon = MonitorFromWindow(GetConsoleWindow(), MONITOR_DEFAULTTOPRIMARY);
            DEVICE_SCALE_FACTOR scaleFactor = DEVICE_SCALE_FACTOR_INVALID;

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

void platform::sendKeyboardEventToHost(void* event)
{
    MSG* msg = (MSG *)event;
    HWND hHostWnd = 0;

    EnumWindows(EnumWindowsProc, (LPARAM)&hHostWnd);

    if (hHostWnd != 0) {
        // XXX This works but it is suboptimal because:
        //   - Host main window is determined using an unreliable method
        //   - Focus is moved from the plugin window to the host window
        //   - JUCE based softsynth OB-Xd does it flawlessly

        SetFocus(hHostWnd);
        PostMessage(hHostWnd, msg->lParam, msg->wParam, 0);
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    DWORD winProcId = 0;
    GetWindowThreadProcessId(hWnd, &winProcId);

    if (winProcId == GetCurrentProcessId()) {
        char text[256];
        GetWindowText(hWnd, (LPSTR)text, sizeof(text));

        if (strstr(text, "Ableton Live") != 0) {
            *((HWND *)lParam) = hWnd;
            return FALSE;
        }
    }

    return TRUE;
}
