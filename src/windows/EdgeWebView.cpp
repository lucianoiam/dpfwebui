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

#include "DistrhoPluginInfo.h"
#include "Platform.hpp"
#include "log.h"

#define _LPCWSTR(s) std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s).c_str()

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView()
    : fController(0)
    , fWindowId(0)
{
    // EdgeWebView works a bit different compared to the other platforms due to
    // the async nature of the native web view initialization process.
}

EdgeWebView::~EdgeWebView()
{
    cleanupWebView();
}

void EdgeWebView::reparent(uintptr_t windowId)
{
    bool isInitializing = fWindowId != 0;
    fWindowId = windowId;

    if (fController == 0) {
        if (!isInitializing) {
            initWebView();
        }

        return; // will come back later
    }

    ICoreWebView2Controller2_put_ParentWindow(fController, (HWND)windowId);
}

void EdgeWebView::navigate(String url)
{
    fUrl = url;

    if (fController == 0) {
        return; // will come back later
    }

    ICoreWebView2* webView;
    ICoreWebView2Controller2_get_CoreWebView2(fController, &webView);
    ICoreWebView2_Navigate(webView, _LPCWSTR(fUrl));
}

void EdgeWebView::resize(const Size<uint>& size)
{
    fSize = size;

    if (fController == 0) {
        return; // will come back later
    }

    RECT bounds {};
    bounds.right = fSize.getWidth();
    bounds.bottom = fSize.getHeight();

    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

void EdgeWebView::initWebView()
{
    // See handleWebViewControllerCompleted() below
    HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0, _LPCWSTR(platform::getTemporaryPath()), 0, this);

    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment", result);
    }
}

void EdgeWebView::cleanupWebView()
{
    if (fController != 0) {
        ICoreWebView2* webView;
        ICoreWebView2Controller2_get_CoreWebView2(fController, &webView);
        ICoreWebView2_remove_NavigationCompleted(webView, fEventToken);
        ICoreWebView2Controller2_Close(fController);
        ICoreWebView2_Release(fController);
    }

    fController = 0;
}

HRESULT EdgeWebView::handleWebViewEnvironmentCompleted(HRESULT result,
                                                       ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment", result);
        return result;
    }

    // TODO: cleanup, explain why using a dummy window instead of parent for this
    WCHAR tempClassName[256];
    ::swprintf(tempClassName, sizeof(tempClassName), L"DPF_Class_%d", std::rand());

    WNDCLASS wc = {};
    wc.lpszClassName = wcsdup(tempClassName);
    wc.lpfnWndProc = DefWindowProc;
    ::RegisterClass(&wc);
    
    HWND dummy = ::CreateWindowEx(
        WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"WebView2 Init",
        WS_POPUPWINDOW | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, 0, 0
    );

    ::ShowWindow(dummy,  SW_SHOWNOACTIVATE);

    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, dummy, this);

    return S_OK;
}

HRESULT EdgeWebView::handleWebViewControllerCompleted(HRESULT result,
                                                      ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 controller", result);
        return result;
    }

    // TODO: there is some visible black flash while the window plugin appears and
    //       Windows' window animations are enabled. Such color is set in pugl_win.cpp:
    // /impl->wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    fController = controller;
    ICoreWebView2Controller2_AddRef(fController);
#ifdef DISTRHO_UI_BACKGROUND_COLOR
    COREWEBVIEW2_COLOR color;
    color.R = DISTRHO_UI_BACKGROUND_COLOR >> 24;
    color.G = (DISTRHO_UI_BACKGROUND_COLOR & 0x00ff0000) >> 16;
    color.B = (DISTRHO_UI_BACKGROUND_COLOR & 0x0000ff00) >> 8;
    color.A = DISTRHO_UI_BACKGROUND_COLOR && 0x000000ff;
    // Not sure about the legality of the cast below
    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);
#endif

    ICoreWebView2* webView;
    ICoreWebView2Controller2_get_CoreWebView2(fController, &webView);
    ICoreWebView2_add_NavigationCompleted(webView, this, &fEventToken);

    resize(fSize);
    navigate(fUrl);

    return S_OK;
}

HRESULT EdgeWebView::handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                                      ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;

    if (fController != 0) {
        reparent(fWindowId);
    }

    return S_OK;
}

void EdgeWebView::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
