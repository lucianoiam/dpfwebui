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

#include "EdgeWebView.hpp"

#include <codecvt>
#include <locale>
#include <sstream>
#include <pathcch.h>
#include <shlobj.h>
#include <shlwapi.h>

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

    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(nullptr, getTempPath().c_str(),
        nullptr, &fHandler);

    if (FAILED(result)) {
        errorMessageBox(L"Failed to create WebView2 environment options", result);
    }
}

void EdgeWebView::cleanup()
{
    if (fController != nullptr) {
        fController->lpVtbl->Close(fController);
    }

    fController = nullptr;
    fView = nullptr;
    fHandler.EnvironmentCompleted = nullptr;
    fHandler.ControllerCompleted = nullptr;
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

void EdgeWebView::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    MessageBox(nullptr, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
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
    // replaced with something else.
    PathCchRemoveExtension(exePath, MAX_PATH);
    wcscat(tempPath, PathFindFileName(exePath));

    return static_cast<const wchar_t *>(tempPath);
}
