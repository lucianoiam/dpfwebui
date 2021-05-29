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

#include <libloaderapi.h>
#include <shlwapi.h>

#include "log.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// Add plugin path to DLL search path so WebView2Loader.dll can be found

BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    char path[MAX_PATH];

    if (GetModuleFileName((HINSTANCE)&__ImageBase, path, sizeof(path)) == 0) {
        LOG_STDERR_INT("Could not determine DLL path", GetLastError());
        return FALSE;
    }

    PathRemoveFileSpec(path);
    AddDllDirectory(path);

    return TRUE;
}
