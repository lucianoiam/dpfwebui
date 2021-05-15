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
#include <tchar.h>

#include "../DistrhoPluginInfo.h"

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView()
    : fController(nullptr)
    , fView(nullptr)
{
    // empty
}

EdgeWebView::~EdgeWebView()
{
    close();
}

void EdgeWebView::reparent(uintptr_t parentWindowId)
{
    // FIXME: Trying to reparent WebView2 calling put_ParentWindow() results in flicker.
    //        Need to recreate all WebView2 objects.

    HWND hWnd = (HWND)parentWindowId;

    close();

    fHandler.EnvironmentCompleted = [&](HRESULT result, ICoreWebView2Environment* createdEnv) {
        if (FAILED(result)) {
            errorMessageBox("Failed to create WebView2 environment", result);
            return result;
        }

        createdEnv->lpVtbl->CreateCoreWebView2Controller(createdEnv, hWnd, &fHandler);

        return S_OK;
    };

    fHandler.ControllerCompleted = [&](HRESULT result, ICoreWebView2Controller* createdController) {
        if (FAILED(result)) {
            errorMessageBox("Failed to create WebView2 controller", result);
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

    // Need to set files location, calling CreateCoreWebView2Environment()
    // fails with HRESULT 0x80070005 E_ACCESSDENIED

    TCHAR szDataPath[MAX_PATH];
    getDataPath(szDataPath, _countof(szDataPath));

    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        //TStrToWStr(szDataPath).c_str(),   // comment out for now
        L"C:\\tmp",     // works
        nullptr,
        &fHandler
    );

    if (FAILED(result)) {
        errorMessageBox("Failed to create WebView2 environment options", result);
    }
}

void EdgeWebView::close()
{
    if (fController == nullptr) {
        return;
    }

    fController->lpVtbl->Close(fController);
}

void EdgeWebView::resize(HWND hWnd)
{
    if (fController == nullptr) {
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

void EdgeWebView::errorMessageBox(std::string message, HRESULT result)
{
    std::stringstream ss;
    ss << message << ", code 0x" << std::hex << result;
    MessageBox(nullptr, ss.str().c_str(), DISTRHO_PLUGIN_NAME, MB_OK | MB_ICONSTOP);
}

void EdgeWebView::getDataPath(LPTSTR szOut, DWORD nSize)
{
    // TODO -- check this
    TCHAR szExePath[MAX_PATH];
    GetModuleFileName(nullptr, szExePath, _countof(szExePath));

    LPTSTR szExeName = _tcsrchr(szExePath, TEXT('\\'));
    szExeName = szExeName ? szExeName + 1 : szExePath;

    TCHAR szAppData[MAX_PATH];
    GetEnvironmentVariable(TEXT("AppData"), szAppData, _countof(szAppData));

    _tcsncpy(szOut, szAppData, nSize);
    _tcsncat(szOut, TEXT("\\"), nSize);
    _tcsncat(szOut, szExeName, nSize);
}
