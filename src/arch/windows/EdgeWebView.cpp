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

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <winuser.h>

#include "base/Platform.hpp"
#include "base/macro.h"
#include "extra/cJSON.h"
#include "DistrhoPluginInfo.h"

#define WSTRING_CONVERTER std::wstring_convert<std::codecvt_utf8<wchar_t>>()
#define TO_LPCWSTR(s)     WSTRING_CONVERTER.from_bytes(s).c_str()
#define TO_LPCSTR(s)      WSTRING_CONVERTER.to_bytes(s).c_str()

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.chrome.webview.postMessage(args);"

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView(WebViewEventHandler& handler)
    : BaseWebView(handler)
    , fStarted(false)
    , fBusy(false)
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
    ::ShowWindow(fHelperHwnd, SW_SHOWNOACTIVATE);
    injectDefaultScripts(String(JS_POST_MESSAGE_SHIM));
}

EdgeWebView::~EdgeWebView()
{
    if (fBusy) {
        ::Sleep(1000); // avoid crash when opening and closing the UI too quickly
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
    if (fController == 0) {
        fPWindowId = windowId;
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
    fBusy = true;
    ICoreWebView2_Navigate(fView, TO_LPCWSTR(url));
}

void EdgeWebView::runScript(String source)
{
    assert(fView != 0); // in the plugin specific case, fView==0 means a programming error
    ICoreWebView2_ExecuteScript(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebView::injectScript(String source)
{
    if (fController == 0) {
        fPInjectedScripts.push_back(source);
        return; // later
    }
    ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, TO_LPCWSTR(source), 0);
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

void EdgeWebView::start()
{
    if (fStarted) {
        return;
    }
    fStarted = true;
    HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0,
        TO_LPCWSTR(platform::getTemporaryPath()), 0, this);
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
    }
}

HRESULT EdgeWebView::handleWebView2EnvironmentCompleted(HRESULT result,
                                                        ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }
    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, this);
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2ControllerCompleted(HRESULT result,
                                                       ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        // Calling MessageBox here makes the host crash if 'this' is being deleted
        DISTRHO_LOG_STDERR_INT("handleWebView2ControllerCompleted() called with HRESULT", result);
        return result;
    }
    // TODO: there is some visible black flash while the window plugin is appearing and
    //       Windows' window animations are enabled. Such color is set in pugl_win.cpp:
    // impl->wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    fController = controller;
    ICoreWebView2Controller2_AddRef(fController);
    //COREWEBVIEW2_COLOR clear = {};
    //ICoreWebView2Controller2_put_DefaultBackgroundColor(
    //    reinterpret_cast<ICoreWebView2Controller2 *>(fController), clear);
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
    fBusy = false;
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                      ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    // Edge WebView2 does not provide access to JavaScriptCore values
    (void)sender;
    LPWSTR jsonStr;
    ICoreWebView2WebMessageReceivedEventArgs_get_WebMessageAsJson(eventArgs, &jsonStr);
    cJSON* jArgs = ::cJSON_Parse(TO_LPCSTR(jsonStr));
    ::CoTaskMemFree(jsonStr);
    ScriptValueVector args;
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
    handleScriptMessage(args);
    return S_OK;
}

void EdgeWebView::webViewLoaderErrorMessageBox(HRESULT result)
{
    // TODO: it would be better to display a message within the window using DPF calls
    std::wstringstream ss;
    ss << "Make sure you have installed the Microsoft Edge WebView2 Runtime. "
       << "Error 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
