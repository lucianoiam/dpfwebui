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

#include <stdio.h>

#include "comutil.h"

LPWSTR refiid2wstr(LPWSTR buf, REFIID riid)
{
    swprintf(buf, 37, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        riid->Data1,
        riid->Data2,
        riid->Data3,
        riid->Data4[0],
        riid->Data4[1],
        riid->Data4[2],
        riid->Data4[3],
        riid->Data4[4],
        riid->Data4[5],
        riid->Data4[6],
        riid->Data4[7]
    );
    return buf;
}
