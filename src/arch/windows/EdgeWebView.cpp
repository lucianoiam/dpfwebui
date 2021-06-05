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
#include <winuser.h>

#include "base/Platform.hpp"
#include "base/log.h"
#include "DistrhoPluginInfo.h"

#define _LPCWSTR(s) std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s).c_str()

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView(WebViewScriptMessageHandler& handler)
    : BaseWebView(handler)
    , fController(0)
    , fView(0)
    , fWindowId(0)
    , fHelperHwnd(0)
{
    // EdgeWebView works a bit different compared to the other platforms due to
    // the async nature of the native web view initialization process
    WCHAR className[256];
    ::swprintf(className, sizeof(className), L"DPF_Class_%d", std::rand());
    ::ZeroMemory(&fHelperClass, sizeof(fHelperClass));
    fHelperClass.lpszClassName = wcsdup(className);
    fHelperClass.lpfnWndProc = DefWindowProc;
    ::RegisterClass(&fHelperClass);
    fHelperHwnd = ::CreateWindowEx(
        WS_EX_TOOLWINDOW,
        fHelperClass.lpszClassName,
        L"WebView2 Init Helper",
        WS_POPUPWINDOW | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, 0, 0
    );
    ::ShowWindow(fHelperHwnd,  SW_SHOWNOACTIVATE);
}

EdgeWebView::~EdgeWebView()
{
    if (fController != 0) {
        ICoreWebView2Controller2_Close(fController);
        ICoreWebView2_Release(fController);
    }
    ::DestroyWindow(fHelperHwnd);
    ::UnregisterClass(fHelperClass.lpszClassName, 0);
    ::free((void*)fHelperClass.lpszClassName);
}

void EdgeWebView::reparent(uintptr_t windowId)
{
    // WebView2 is created here if needed
    bool isInitializing = fWindowId != 0;
    fWindowId = windowId;
    if (fController == 0) {
        if (!isInitializing) {
            // See handleWebViewControllerCompleted()
            HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0,
                _LPCWSTR(platform::getTemporaryPath()), 0, this);
            if (FAILED(result)) {
                errorMessageBox(L"Could not create WebView2 environment", result);
            }
        }
        return; // later
    }
    ICoreWebView2Controller2_put_ParentWindow(fController, (HWND)windowId);
}

void EdgeWebView::navigate(String url)
{
    fUrl = url;
    if (fView == 0) {
        return; // later
    }
    ICoreWebView2_Navigate(fView, _LPCWSTR(fUrl));
}

void EdgeWebView::runScript(String source)
{
    ICoreWebView2_ExecuteScript(fView, _LPCWSTR(source), 0);
}

void EdgeWebView::injectScript(String source)
{
    fInjectedJs.push_back(source);
}

void EdgeWebView::resize(const Size<uint>& size)
{
    fSize = size;
    if (fController == 0) {
        return; // later
    }
    RECT bounds {};
    bounds.right = fSize.getWidth();
    bounds.bottom = fSize.getHeight();
    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

HRESULT EdgeWebView::handleWebViewEnvironmentCompleted(HRESULT result,
                                                       ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 environment", result);
        return result;
    }
    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, this);
    return S_OK;
}

HRESULT EdgeWebView::handleWebViewControllerCompleted(HRESULT result,
                                                      ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        errorMessageBox(L"Could not create WebView2 controller", result);
        return result;
    }
    // TODO: there is some visible black flash while the window plugin is appearing and
    //       Windows' window animations are enabled. Such color is set in pugl_win.cpp:
    // impl->wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    fController = controller;
    ICoreWebView2Controller2_AddRef(fController);
    COREWEBVIEW2_COLOR clear = {};
    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), clear);
    ICoreWebView2Controller2_get_CoreWebView2(fController, &fView);
    ICoreWebView2_add_NavigationCompleted(fView, this, 0);
    for (std::vector<String>::iterator it = fInjectedJs.begin(); it != fInjectedJs.end(); ++it) {
        ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, _LPCWSTR(*it), 0);
    }
    // Call pending setters
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
        loadFinished();
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
