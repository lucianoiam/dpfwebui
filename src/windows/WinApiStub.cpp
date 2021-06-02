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

#include "WinApiStub.hpp"

#include <libloaderapi.h>

// Explanation for the GCC warnings https://github.com/chriskohlhoff/asio/issues/631
typedef HRESULT GETPROCESSDPIAWARENESS(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
typedef HRESULT GETSCALEFACTORFORMONITOR(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

HRESULT stub::GetProcessDpiAwareness(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue)
{
    GETPROCESSDPIAWARENESS *f = (GETPROCESSDPIAWARENESS*)GetProcAddressForDllName(
    	"Shcore.dll", "GetProcessDpiAwareness");
    if (f == 0) {
        return ::GetLastError();
    }
    return (*f)(hProc, pValue);
}

HRESULT stub::GetScaleFactorForMonitor(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale)
{
    GETSCALEFACTORFORMONITOR *f = (GETSCALEFACTORFORMONITOR*)GetProcAddressForDllName(
    	"Shcore.dll", "GetScaleFactorForMonitor");
    if (f == 0) {
        return ::GetLastError();
    }
    return (*f)(hMon, pScale);
}

FARPROC stub::GetProcAddressForDllName(LPCSTR lpDllName, LPCSTR lpProcName)
{
    HMODULE hm = ::LoadLibrary(lpDllName);
    if (hm == 0) {
        return 0;
    }

    return ::GetProcAddress(hm, lpProcName);
}
