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

#define UNICODE
#include <libloaderapi.h>
#include <shlwapi.h>

#include "../log.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// Add plugin path to DLL search path so WebView2Loader.dll can be found

BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    WCHAR dllPath[MAX_PATH];

    if (GetModuleFileName((HINSTANCE)&__ImageBase, dllPath, sizeof(dllPath)) == 0) {
        LOG_STDERR_INT("Failed GetModuleFileName() call", GetLastError());
        return FALSE;
    }

    PathRemoveFileSpec(dllPath);
    AddDllDirectory(dllPath);

    return TRUE;
}
