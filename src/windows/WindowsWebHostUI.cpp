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

#include <shellscalingapi.h>

#include "WindowsWebHostUI.hpp"

USE_NAMESPACE_DISTRHO

WindowsWebHostUI::WindowsWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
{
    if (shouldCreateWebView()) {
        setWebView(new EdgeWebView()); // base class owns web view
    }
}

WindowsWebHostUI::~WindowsWebHostUI()
{
    // TODO - standalone support
}

float WindowsWebHostUI::getDisplayScaleFactor(uintptr_t window)
{
    float k = 1.f;
    const HMODULE hm = LoadLibrary("Shcore.dll");

    if (hm == 0) {
        return k;
    }

    typedef HRESULT (*PFN_GetProcessDpiAwareness)(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
    typedef HRESULT (*PFN_GetScaleFactorForMonitor)(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
    const PFN_GetProcessDpiAwareness GetProcessDpiAwareness
        = (PFN_GetProcessDpiAwareness)GetProcAddress(hm, "GetProcessDpiAwareness");
    const PFN_GetScaleFactorForMonitor GetScaleFactorForMonitor
        = (PFN_GetScaleFactorForMonitor)GetProcAddress(hm, "GetScaleFactorForMonitor");
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif

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

void WindowsWebHostUI::openSystemWebBrowser(String& url)
{
    ShellExecute(0, "open", url.buffer(), 0, 0, SW_SHOWNORMAL);
}

uintptr_t WindowsWebHostUI::createStandaloneWindow()
{
    // TODO - standalone support
    return 0;
}

void WindowsWebHostUI::processStandaloneEvents()
{
    // TODO - standalone support
}
