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

#include "EdgeWebViewUI.hpp"

#include <codecvt>
#include <locale>
#include <sstream>
#include <libloaderapi.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "DistrhoPluginInfo.h"
#include "log.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new EdgeWebViewUI;
}

EdgeWebViewUI::EdgeWebViewUI()
    : fController(0)
    , fView(0)
{
    // empty
}

EdgeWebViewUI::~EdgeWebViewUI()
{
    cleanup();
}

void EdgeWebViewUI::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}

void EdgeWebViewUI::reparent(uintptr_t windowId)
{
    // FIXME: Trying to reparent WebView2 calling controller.put_ParentWindow() results in heavy
    //        flicker to the point the view is unusable. Need to reinitialize everything (@#!)

    HWND hWnd = (HWND)windowId;

    cleanup();

    fHandler.EnvironmentCompleted = [&](HRESULT result, ICoreWebView2Environment* createdEnv) {
        if (FAILED(result)) {
            errorMessageBox(L"Failed to create WebView2 environment", result);
            return result;
        }

        createdEnv->lpVtbl->CreateCoreWebView2Controller(createdEnv, hWnd, &fHandler);

        return S_OK;
    };

    fHandler.ControllerCompleted = [&](HRESULT result, ICoreWebView2Controller* createdController) {
        if (FAILED(result)) {
            errorMessageBox(L"Failed to create WebView2 controller", result);
            return result;
        }

        fController = createdController;
        fController->lpVtbl->AddRef(fController);
        fController->lpVtbl->get_CoreWebView2(fController, &fView);
        fView->lpVtbl->AddRef(fView);

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring url = converter.from_bytes(getContentUrl());
        fView->lpVtbl->Navigate(fView, url.c_str());

        resize(hWnd);

        return S_OK;
    };

    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(0, getTempPath().c_str(), 0, &fHandler);

    if (FAILED(result)) {
        errorMessageBox(L"Failed to create WebView2 environment options", result);
    }
}

void EdgeWebViewUI::cleanup()
{
    if (fController != 0) {
        fController->lpVtbl->Close(fController);
    }

    fController = 0;
    fView = 0;
    fHandler.EnvironmentCompleted = 0;
    fHandler.ControllerCompleted = 0;
}

void EdgeWebViewUI::resize(HWND hWnd)
{
    if (fController == 0) {
        return;
    }

    RECT bounds;
    GetClientRect(hWnd, &bounds);
    
    // FIXME hWnd bounds seem to be incorrect for example 0;31;-1670580753;32767
    bounds.top = 0;
    bounds.left = 0;
    bounds.right = 800;
    bounds.bottom = 600;

    fController->lpVtbl->put_Bounds(fController, bounds);
}

void EdgeWebViewUI::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}

std::wstring EdgeWebViewUI::getTempPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPFTemp
    WCHAR tempPath[MAX_PATH];
    HRESULT result = SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_DEFAULT, tempPath);
    if (FAILED(result)) {
        LOG_STDERR_INT("Failed SHGetFolderPath() call", result);
        return {};
    }

    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously. C:\Users\< USERNAME >\AppData\Local\DPFTemp\< HOST_BIN >
    WCHAR exePath[MAX_PATH];
    if (GetModuleFileName(NULL, exePath, sizeof(exePath)) == 0) {
        LOG_STDERR_INT("Failed GetModuleFileName() call", GetLastError());
        return {};
    }

    wcscat(tempPath, L"\\DPFTemp\\");

    // The following call relies on a further Windows library called Pathcch, which is implemented
    // in in api-ms-win-core-path-l1-1-0.dll and requires Windows 8. Min plugin target is Windows 7.
    //PathCchRemoveExtension(exePath, sizeof(exePath));
    wcscat(tempPath, PathFindFileName(exePath));

    return static_cast<const wchar_t *>(tempPath);
}
