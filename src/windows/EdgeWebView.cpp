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

#include "EdgeWebView.hpp"

#include <codecvt>
#include <locale>
#include <sstream>
#include <shlobj.h>
#include <shlwapi.h>

#include "../DistrhoPluginInfo.h"

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new EdgeWebView;
}

EdgeWebView::EdgeWebView()
    : fController(0)
    , fView(0)
{
    // empty
}

EdgeWebView::~EdgeWebView()
{
    cleanup();
}

void EdgeWebView::reparent(uintptr_t parentWindowId)
{
    // FIXME: Trying to reparent WebView2 calling controller.put_ParentWindow() results in heavy
    //        flicker to the point the view is unusable. Need to reinitialize everything (@#!)

    HWND hWnd = (HWND)parentWindowId;

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

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
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

void EdgeWebView::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}

void EdgeWebView::cleanup()
{
    if (fController != 0) {
        fController->lpVtbl->Close(fController);
    }

    fController = 0;
    fView = 0;
    fHandler.EnvironmentCompleted = 0;
    fHandler.ControllerCompleted = 0;
}

void EdgeWebView::resize(HWND hWnd)
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

void EdgeWebView::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}

std::wstring EdgeWebView::getTempPath()
{
    // Get temp path inside user files folder: C:\Users\< USERNAME >\AppData\Local\DPFTemp
    WCHAR tempPath[MAX_PATH + 1];
    SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_DEFAULT, tempPath);
    wcscat(tempPath, L"\\DPFTemp\\");

    // Append host executable name to the temp path otherwise WebView2 controller initialization
    // fails with HRESULT 0x8007139f when trying to load plugin into more than a single host
    // simultaneously. C:\Users\< USERNAME >\AppData\Local\DPFTemp\< BIN >
    WCHAR exePath[MAX_PATH + 1];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // The following call relies on a further Windows library called Pathcch, maybe it should be
    // replaced with something else. Implemented in api-ms-win-core-path-l1-1-0.dll which requires
    // Windows 8
    //PathCchRemoveExtension(exePath, MAX_PATH);
    wcscat(tempPath, PathFindFileName(exePath));

    return static_cast<const wchar_t *>(tempPath);
}
