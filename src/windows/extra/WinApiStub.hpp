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

#ifndef WINAPI_STUB_HPP
#define WINAPI_STUB_HPP

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

} // namespace stub

#endif // WINAPI_STUB_HPP
