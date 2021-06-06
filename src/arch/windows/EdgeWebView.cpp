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

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <winuser.h>

#include "base/Platform.hpp"
#include "base/log.h"
#include "extra/cJSON.h"
#include "DistrhoPluginInfo.h"

#define _LPCWSTR(s) std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s).c_str()
#define _LPCSTR(s)  std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s).c_str()
#define HOST_SHIM_JS "window.webviewHost = {postMessage: (args) => window.chrome.webview.postMessage(args)};"

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView(WebViewEventHandler& handler)
    : BaseWebView(handler)
    , fWebViewBusy(false)
    , fHelperHwnd(0)
    , fController(0)
    , fView(0)
    , fPWindowId(0)
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
    
    injectDefaultScripts(String(HOST_SHIM_JS));
}

EdgeWebView::~EdgeWebView()
{
    if (fWebViewBusy) {
         // Avoid crash when opening and closing the UI too quickly
        ::Sleep(1000);
    }
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
    // WebView2 is created during reparent() if needed
    if (fController == 0) {
        bool isInitializing = fPWindowId != 0;
        fPWindowId = windowId;
        if (!isInitializing) {
            fWebViewBusy = true;
            // See handleWebViewControllerCompleted() below
            HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0,
                _LPCWSTR(platform::getTemporaryPath()), 0, this);
            if (FAILED(result)) {
                // FIXME: "make sure runtime is installed..."
                errorMessageBox(L"Could not create WebView2 environment", result);
            }
        }
        return; // later
    }
    ICoreWebView2Controller2_put_ParentWindow(fController, (HWND)windowId);
}

void EdgeWebView::navigate(String url)
{
    if (fView == 0) {
        fPUrl = url;
        return; // later
    }
    fWebViewBusy = true;
    ICoreWebView2_Navigate(fView, _LPCWSTR(url));
}

void EdgeWebView::runScript(String source)
{
    assert(fView != 0); // in the plugin specific case, fView==0 means a programming error
    ICoreWebView2_ExecuteScript(fView, _LPCWSTR(source), 0);
}

void EdgeWebView::injectScript(String source)
{
    if (fController == 0) {
        fPInjectedScripts.push_back(source);
        return; // later
    }
    ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, _LPCWSTR(source), 0);
}

void EdgeWebView::resize(const Size<uint>& size)
{
    if (fController == 0) {
        fPSize = size;
        return; // later
    }
    RECT bounds {};
    bounds.right = size.getWidth();
    bounds.bottom = size.getHeight();
    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

HRESULT EdgeWebView::handleWebView2EnvironmentCompleted(HRESULT result,
                                                        ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        // FIXME: "make sure runtime is installed..."
        errorMessageBox(L"Could not create WebView2 environment", result);
        return result;
    }
    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, this);
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2ControllerCompleted(HRESULT result,
                                                       ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        LOG_STDERR_INT("handleWebView2ControllerCompleted() called with HRESULT", result);
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
    ICoreWebView2_add_WebMessageReceived(fView, this, 0);
    // Call pending setters
    for (std::vector<String>::iterator it = fPInjectedScripts.begin(); it != fPInjectedScripts.end(); ++it) {
        injectScript(*it);
    }
    resize(fPSize);
    navigate(fPUrl);
    // Cleanup, handleWebViewControllerCompleted() will not be called again anyways
    fPInjectedScripts.clear();
    fPSize = {};
    fPUrl.clear();
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                                       ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;
    if (fController != 0) {
        handleLoadFinished();
        if (fPWindowId != 0) {
            reparent(fPWindowId);
            // handleWebViewNavigationCompleted() could be called again
            fPWindowId = 0;
        }
    }
    fWebViewBusy = false;
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                      ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    // Edge WebView2 does not provide access to JavaScriptCore values
    (void)sender;
    LPWSTR jsonStr;
    ICoreWebView2WebMessageReceivedEventArgs_get_WebMessageAsJson(eventArgs, &jsonStr);
    cJSON* jArgs = ::cJSON_Parse(_LPCSTR(jsonStr));
    ScriptMessageArguments args;
    if (::cJSON_IsArray(jArgs)) {
        int numArgs = ::cJSON_GetArraySize(jArgs);
        if (numArgs > 0) {
            for (int i = 0; i < numArgs; i++) {
                cJSON* jArg = ::cJSON_GetArrayItem(jArgs, i);
                if (::cJSON_IsFalse(jArg)) {
                    args.push_back(ScriptValue(false));
                } else if (::cJSON_IsTrue(jArg)) {
                    args.push_back(ScriptValue(true));
                } else if (::cJSON_IsNumber(jArg)) {
                    args.push_back(ScriptValue(::cJSON_GetNumberValue(jArg)));
                } else if (::cJSON_IsString(jArg)) {
                    args.push_back(ScriptValue(String(::cJSON_GetStringValue(jArg))));
                } else {
                    args.push_back(ScriptValue()); // null
                }
            }
        }
    }
    ::cJSON_free(jArgs);
    ::CoTaskMemFree(jsonStr);
    handleScriptMessage(args);
    return S_OK;
}

void EdgeWebView::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
