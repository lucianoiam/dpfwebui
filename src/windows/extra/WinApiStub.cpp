/*
 * Apices - Audio Plugins In C++ & ES6
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

#include "WinApiStub.hpp"

#include <libloaderapi.h>

// Explanation for the GCC warnings https://github.com/chriskohlhoff/asio/issues/631
typedef HRESULT GETPROCESSDPIAWARENESS(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
typedef HRESULT GETSCALEFACTORFORMONITOR(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

HRESULT stub::GetProcessDpiAwareness(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue)
{
    HRESULT hr;
    HMODULE hm = LoadLibrary("Shcore.dll");

    if (hm != 0) {
        GETPROCESSDPIAWARENESS *f = (GETPROCESSDPIAWARENESS*)GetProcAddress(hm,
            "GetProcessDpiAwareness");

        if (f != 0) {
            hr = (*f)(hProc, pValue);
        } else {
            hr = GetLastError();
        }

        FreeLibrary(hm);
    } else {
        hr = GetLastError();
    }

    return hr;
}

HRESULT stub::GetScaleFactorForMonitor(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale)
{
    HRESULT hr;
    HMODULE hm = LoadLibrary("Shcore.dll");

    if (hm != 0) {
        GETSCALEFACTORFORMONITOR *f = (GETSCALEFACTORFORMONITOR*)GetProcAddress(hm,
            "GetScaleFactorForMonitor");

        if (f != 0) {
            hr = (*f)(hMon, pScale);
        } else {
            hr = GetLastError();
        }

        FreeLibrary(hm);
    } else {
        hr = GetLastError();
    }

    return hr;
}
