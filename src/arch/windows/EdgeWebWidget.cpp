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

#include "EdgeWebWidget.hpp"

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <winuser.h>

#include "base/Platform.hpp"
#include "base/macro.h"
#include "extra/cJSON.h"

#include "DistrhoPluginInfo.h"

#define WSTR_CONVERTER std::wstring_convert<std::codecvt_utf8<wchar_t>>()
#define TO_LPCWSTR(s)  WSTR_CONVERTER.from_bytes(s).c_str()
#define TO_LPCSTR(s)   WSTR_CONVERTER.to_bytes(s).c_str()

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.chrome.webview.postMessage(args);"

USE_NAMESPACE_DISTRHO

EdgeWebWidget::EdgeWebWidget(Window& windowToMapTo)
    : BaseWebWidget(windowToMapTo)
    , fHelperHwnd(0)
    , fDisplayed(false)
    , fBackgroundColor(0)
    , fHandler(0)
    , fController(0)
    , fView(0)
{
    // EdgeWebWidget works a bit different compared to the other platforms due
    // to the async nature of the native web view initialization process
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
    reparent(windowToMapTo); // queue request
    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
    fHandler = new InternalWebView2EventHandler(this);
}

EdgeWebWidget::~EdgeWebWidget()
{
    fHandler->release();
    if (fController != 0) {
        ICoreWebView2Controller2_Close(fController);
        ICoreWebView2_Release(fController);
    }
    ::DestroyWindow(fHelperHwnd);
    ::UnregisterClass(fHelperClass.lpszClassName, 0);
    ::free((void*)fHelperClass.lpszClassName);
}

void EdgeWebWidget::onDisplay()
{
    if (fDisplayed) {
        return;
    }
    fDisplayed = true;
    // "onVisibilityChanged(true)"
    initWebView();
}

void EdgeWebWidget::onResize(const ResizeEvent& ev)
{
    if (fController == 0) {
        return; // later
    }
    updateWebViewSize(ev.size);
}

void EdgeWebWidget::setBackgroundColor(uint32_t rgba)
{
    if (fController == 0) {
        fBackgroundColor = rgba;
        return; // later
    }
    // WebView2 currently only supports alpha=0 or alpha=1
    COREWEBVIEW2_COLOR color;
    color.A = static_cast<BYTE>(rgba & 0x000000ff);
    color.R = static_cast<BYTE>(rgba >> 24);
    color.G = static_cast<BYTE>((rgba & 0x00ff0000) >> 16);
    color.B = static_cast<BYTE>((rgba & 0x0000ff00) >> 8 );
    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);
}

void EdgeWebWidget::reparent(Window& windowToMapTo)
{
    if (fController == 0) {
        return; // later
    }
    HWND hWnd = reinterpret_cast<HWND>(windowToMapTo.getNativeWindowHandle());
    ICoreWebView2Controller2_put_ParentWindow(fController, hWnd);
}

void EdgeWebWidget::navigate(String& url)
{
    if (fView == 0) {
        fUrl = url;
        return; // later
    }
    ICoreWebView2_Navigate(fView, TO_LPCWSTR(url));
}

void EdgeWebWidget::runScript(String& source)
{
    // For the plugin specific use case fView==0 means a programming error.
    // There is no point in queuing these, just wait for the view to become
    // ready before trying to run scripts.
    assert(fView != 0);
    ICoreWebView2_ExecuteScript(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebWidget::injectScript(String& source)
{
    if (fController == 0) {
        fInjectedScripts.push_back(source);
        return; // later
    }
    ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebWidget::updateWebViewSize(Size<uint> size)
{
    RECT bounds {};
    bounds.right = size.getWidth();
    bounds.bottom = size.getHeight();
    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

void EdgeWebWidget::initWebView()
{
    HRESULT result = ::CreateCoreWebView2EnvironmentWithOptions(0,
        TO_LPCWSTR(platform::getTemporaryPath()), 0, fHandler);
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
    }
}

HRESULT EdgeWebWidget::handleWebView2EnvironmentCompleted(HRESULT result,
                                                        ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }
    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, fHandler);
    // FIXME: handleWebView2ControllerCompleted() is never called when running standalone
    //        unless the app window border is clicked, looks like messages get stuck somewhere
    //        and does not seem related to the usage of the fHelperHwnd
    return S_OK;
}

HRESULT EdgeWebWidget::handleWebView2ControllerCompleted(HRESULT result,
                                                       ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }
    fController = controller;
    ICoreWebView2Controller2_AddRef(fController);
    ICoreWebView2Controller2_get_CoreWebView2(fController, &fView);
    ICoreWebView2_add_NavigationCompleted(fView, fHandler, 0);
    ICoreWebView2_add_WebMessageReceived(fView, fHandler, 0);
    // Call pending setters
    for (std::vector<String>::iterator it = fInjectedScripts.begin(); it != fInjectedScripts.end(); ++it) {
        injectScript(*it);
    }
    setBackgroundColor(fBackgroundColor);
    // FIXME - for some reason getSize() returns {0,0} only on Windows. Only on
    //         Windows we have UI_TYPE=cairo, is it a TopLevelWidget/Cairo bug?
    updateWebViewSize(getWindow().getSize());
    navigate(fUrl);
    // Cleanup, handleWebViewControllerCompleted() will not be called again anyways
    fInjectedScripts.clear();
    fBackgroundColor = 0;
    fUrl.clear();
    return S_OK;
}

HRESULT EdgeWebWidget::handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                                       ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;
    if (fController != 0) {
        handleLoadFinished();
        reparent(getWindow());
    }
    return S_OK;
}

HRESULT EdgeWebWidget::handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                      ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    // Edge WebView2 does not provide access to JSCore values; resort to parsing JSON
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

void EdgeWebWidget::webViewLoaderErrorMessageBox(HRESULT result)
{
    // TODO: Add clickable link to installer. It would be also better to display
    // a message and button in the native window using DPF drawing methods.
    std::wstringstream wss;
    wss << "Make sure you have installed the Microsoft Edge WebView2 Runtime. "
        << "Error 0x" << std::hex << result;
    std::wstring ws = wss.str();
    DISTRHO_LOG_STDERR_COLOR(TO_LPCSTR(ws));
    ::MessageBox(0, ws.c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
