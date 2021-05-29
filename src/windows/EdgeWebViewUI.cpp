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

#include "EdgeWebViewUI.hpp"

#include <codecvt>
#include <locale>
#include <sstream>

#include "DistrhoPluginInfo.h"
#include "RuntimePath.hpp"
#include "log.h"

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new EdgeWebViewUI;
}

EdgeWebViewUI::EdgeWebViewUI()
    : fController(0)
    , fView(0)
{}

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
            errorMessageBox(L"Could not create WebView2 environment", result);
            return result;
        }

        ICoreWebView2Environment_CreateCoreWebView2Controller(createdEnv, hWnd, &fHandler);

        return S_OK;
    };

    fHandler.ControllerCompleted = [&](HRESULT result, ICoreWebView2Controller* createdController) {
        if (FAILED(result)) {
            errorMessageBox(L"Could not create WebView2 controller", result);
            return result;
        }

        fController = createdController;
        ICoreWebView2Controller2_AddRef(fController);
        ICoreWebView2Controller2_get_CoreWebView2(fController, &fView);
        ICoreWebView2_AddRef(fView);

        // Not sure about the legality of the cast below
        uint rgba = getBackgroundColor();
        COREWEBVIEW2_COLOR color;
        color.R = rgba >> 24;
        color.G = (rgba & 0x00ff0000) >> 16;
        color.B = (rgba & 0x0000ff00) >> 8;
        color.A = rgba & 0x000000ff;
        ICoreWebView2Controller2_put_DefaultBackgroundColor(
        	reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring url = converter.from_bytes(getContentUrl());
        ICoreWebView2_Navigate(fView, url.c_str());

        resize(hWnd);

        return S_OK;
    };

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring temp = converter.from_bytes(rtpath::getTemporaryPath());
    HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0, temp.c_str(), 0, &fHandler);

    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment options", result);
    }
}

void EdgeWebViewUI::cleanup()
{
    if (fController != 0) {
        ICoreWebView2Controller2_Close(fController);
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
    ::GetClientRect(hWnd, &bounds);
    
    // FIXME hWnd bounds seem to be incorrect for example 0;31;-1670580753;32767
    bounds.top = 0;
    bounds.left = 0;
    bounds.right = 800;
    bounds.bottom = 600;

    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

void EdgeWebViewUI::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
