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

#ifndef WINAPISTUB_HPP
#define WINAPISTUB_HPP

#include <shellscalingapi.h>
#include <shtypes.h>

/*
   MinGW is currently unable to find GetProcessDpiAwareness() and GetScaleFactorForMonitor()
   despite #include <shellscalingapi.h> (May '21). Also those require Windows 8.1 and the
   plugin minimum target is Windows 7.
 */

namespace stub {

    HRESULT GetProcessDpiAwareness(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
    HRESULT GetScaleFactorForMonitor(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

    // This is a custom helper function with a WinAPI-like name
    FARPROC GetProcAddressForDllName(LPCSTR lpDllName, LPCSTR lpProcName);

} // namespace stub

#endif // WINAPISTUB_HPP
