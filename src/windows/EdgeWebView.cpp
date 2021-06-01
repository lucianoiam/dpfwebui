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

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView()
    : fController(0)
{
    // Creating an Edge WebView2 requires a HWND but parent window is not available in ctor.
    // EdgeWebView works a bit different compared to the other platforms due to the async nature
    // of the native web view initialization process.
}

EdgeWebView::~EdgeWebView()
{
    cleanupWebView();
}

void EdgeWebView::navigate(String url)
{
    fUrl = url; // queue
}

void EdgeWebView::reparent(uintptr_t windowId)
{
	// The previous parent window is gone, fully reinit WebView2
	// TODO: give put_ParentWindow another try, caused heavy flicker on first attempt
    fWindowId = windowId; // queue
    
    cleanupWebView();

    // See handleWebViewControllerCompleted()
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring temp = converter.from_bytes(platform::getTemporaryPath());
    HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0, temp.c_str(), 0, this);

    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment options", result);
    }
}

void EdgeWebView::resize(const Size<uint>& size)
{
    if (fController == 0) {
        fSize = size; // queue
        return;
    }

    RECT bounds;
    bounds.top = 0;
    bounds.left = 0;
    bounds.right = size.getWidth();
    bounds.bottom = size.getHeight();

    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

HRESULT EdgeWebView::handleWebViewEnvironmentCompleted(HRESULT result,
                                                       ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment", result);
        return result;
    }

    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, (HWND)fWindowId, this);

    return S_OK;
}

HRESULT EdgeWebView::handleWebViewControllerCompleted(HRESULT result,
                                                      ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 controller", result);
        return result;
    }

    fController = controller;
    ICoreWebView2Controller2_AddRef(fController);
    ICoreWebView2Controller2_put_IsVisible(fController, false);

#ifdef DISTRHO_UI_BACKGROUND_COLOR
    COREWEBVIEW2_COLOR color;
    color.A = 0xff; // alpha does not seem to work
    color.R = DISTRHO_UI_BACKGROUND_COLOR >> 24;
    color.G = (DISTRHO_UI_BACKGROUND_COLOR & 0x00ff0000) >> 16;
    color.B = (DISTRHO_UI_BACKGROUND_COLOR & 0x0000ff00) >> 8;
    // Not sure about the legality of the cast below
    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);
#endif

    ICoreWebView2* webView;
    ICoreWebView2Controller2_get_CoreWebView2(fController, &webView);
    ICoreWebView2_AddRef(webView);
    ICoreWebView2_add_NavigationCompleted(webView, this, /* token */0);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring url = converter.from_bytes(fUrl);
    ICoreWebView2_Navigate(webView, url.c_str());

    resize(fSize);

    return S_OK;
}

HRESULT EdgeWebView::handleWebViewNavigationCompleted(ICoreWebView2 *sender,
                                                      ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;
    ICoreWebView2Controller2_put_IsVisible(fController, true);
    return S_OK;
}

void EdgeWebView::cleanupWebView()
{
    if (fController != 0) {
        ICoreWebView2Controller2_Close(fController);
    }

    fController = 0;
}

void EdgeWebView::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
