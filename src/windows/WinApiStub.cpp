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

#include "WinApiStub.hpp"

#include <libloaderapi.h>

// https://github.com/chriskohlhoff/asio/issues/631
typedef HRESULT GETPROCESSDPIAWARENESS(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
typedef HRESULT GETSCALEFACTORFORMONITOR(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

USE_NAMESPACE_DISTRHO

HRESULT stub::GetProcessDpiAwareness(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue)
{
    GETPROCESSDPIAWARENESS *f = (GETPROCESSDPIAWARENESS*)GetProcAddress("Shcore.dll",
        "GetProcessDpiAwareness");
    if (f == 0) {
        return GetLastError();
    }
    return (*f)(hProc, pValue);
}

HRESULT stub::GetScaleFactorForMonitor(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale)
{
    GETSCALEFACTORFORMONITOR *f = (GETSCALEFACTORFORMONITOR*)GetProcAddress("Shcore.dll",
        "GetScaleFactorForMonitor");
    if (f == 0) {
        return GetLastError();
    }
    return (*f)(hMon, pScale);
}

FARPROC stub::GetProcAddress(LPCSTR lpDllName, LPCSTR lpProcName)
{
    HMODULE hm = ::LoadLibrary(lpDllName);
    if (hm == 0) {
        return 0;
    }

    return ::GetProcAddress(hm, lpProcName);
}
