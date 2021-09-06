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
#include <shellapi.h>
#include <winuser.h>

#include "Path.hpp"
#include "macro.h"
#include "cJSON.h"

#include "KeyboardRouter.hpp"
#include "DistrhoPluginInfo.h"

#define WSTR_CONVERTER std::wstring_convert<std::codecvt_utf8<wchar_t>>()
#define TO_LPCWSTR(s)  WSTR_CONVERTER.from_bytes(s).c_str()
#define TO_LPCSTR(s)   WSTR_CONVERTER.to_bytes(s).c_str()

#define JS_POST_MESSAGE_SHIM  "window.webviewHost.postMessage = (args) => window.chrome.webview.postMessage(args);"

#define WEBVIEW2_DOWNLOAD_URL "https://developer.microsoft.com/en-us/microsoft-edge/webview2/#download-section"

LRESULT CALLBACK HelperWindowProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    if (umsg == WM_ERASEBKGND) {
        uint32_t rgba = (uint32_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        if (rgba != 0x000000ff) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            COLORREF bgr = ((rgba & 0xff000000) >> 24) | ((rgba & 0x00ff0000) >> 8)
                            | ((rgba & 0x0000ff00) << 8);
            SetBkColor((HDC)wParam, bgr);
            ExtTextOut((HDC)wParam, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
        }

        return 1;
    }

    return DefWindowProc(hwnd, umsg, wParam, lParam);
}

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView()
    : fHelperHwnd(0)
    , fHandler(0)
    , fController(0)
    , fView(0)
{
    WCHAR className[256];
    swprintf(className, sizeof(className), L"EdgeWebView_%s_%d", XSTR(HIPHOP_PROJECT_ID_HASH), std::rand());
    ZeroMemory(&fHelperClass, sizeof(fHelperClass));
    fHelperClass.cbSize = sizeof(WNDCLASSEX);
    fHelperClass.cbWndExtra = 2 * sizeof(LONG_PTR);
    fHelperClass.lpszClassName = wcsdup(className);
    fHelperClass.lpfnWndProc = HelperWindowProc;
    RegisterClassEx(&fHelperClass);
    fHelperHwnd = CreateWindowEx(0, fHelperClass.lpszClassName, L"EdgeWebView Helper",
                                    WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
    SetWindowLongPtr(fHelperHwnd, GWLP_USERDATA, 0x000000ff);
    ShowWindow(fHelperHwnd, SW_SHOW);

    setKeyboardFocus(false);
    KeyboardRouter::getInstance().incRefCount();

    fHandler = new InternalWebView2EventHandler(this);

    // This request is queued until Edge WebView2 initializes itself
    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);

    // Initialize Edge WebView2. Make sure COM is initialized - 0x800401F0 CO_E_NOTINITIALIZED
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(0,
                        TO_LPCWSTR(path::getTemporaryPath()), 0, fHandler);
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
    }
}

EdgeWebView::~EdgeWebView()
{
    fHandler->release();

    KeyboardRouter::getInstance().decRefCount();

    if (fController != 0) {
        ICoreWebView2Controller2_Close(fController);
        ICoreWebView2Controller2_Release(fController);
    }

    DestroyWindow(fHelperHwnd);
    UnregisterClass(fHelperClass.lpszClassName, 0);
    free((void*)fHelperClass.lpszClassName);
}

void EdgeWebView::realize()
{
    SetParent(fHelperHwnd, (HWND)getParent());
    SetWindowLongPtr(fHelperHwnd, GWLP_USERDATA, (LONG_PTR)getBackgroundColor());
    RedrawWindow(fHelperHwnd, 0, 0, RDW_ERASE);
}

void EdgeWebView::navigate(String& url)
{
    fUrl = url;

    if (fView == 0) {
        return; // queue
    }

    ICoreWebView2_Navigate(fView, TO_LPCWSTR(url));
}

void EdgeWebView::runScript(String& source)
{
    // For the plugin specific use case fView==0 means a programming error.
    // There is no point in queuing these, just wait for the view to load its
    // contents before trying to run scripts. Otherwise use injectScript().
    assert(fView != 0);
    ICoreWebView2_ExecuteScript(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebView::injectScript(String& source)
{
    if (fController == 0) {
        fInjectedScripts.push_back(source);
        return; // queue
    }

    ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebView::onSize(uint width, uint height)
{
    SetWindowPos(fHelperHwnd, 0, 0, 0, width, height, SWP_NOOWNERZORDER | SWP_NOMOVE);

    if (fController != 0) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = static_cast<LONG>(width);
        bounds.bottom = static_cast<LONG>(height);

        ICoreWebView2Controller2_put_Bounds(fController, bounds);
    }
}

void EdgeWebView::onKeyboardFocus(bool focus)
{
    // Allow KeyboardRouter to read focus state
    SetWindowLongPtr(fHelperHwnd, GWLP_USERDATA + 1, (LONG_PTR)focus);
}

HRESULT EdgeWebView::handleWebView2EnvironmentCompleted(HRESULT result,
                                                        ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }

    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, fHandler);

    return S_OK;
}

HRESULT EdgeWebView::handleWebView2ControllerCompleted(HRESULT result,
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

    // Run pending requests. Edge WebView2 currently only supports alpha=0 or alpha=1.

    uint32_t rgba = getBackgroundColor();
    COREWEBVIEW2_COLOR color;
    
    color.A = static_cast<BYTE>(rgba & 0x000000ff);
    color.R = static_cast<BYTE>(rgba >> 24);
    color.G = static_cast<BYTE>((rgba & 0x00ff0000) >> 16);
    color.B = static_cast<BYTE>((rgba & 0x0000ff00) >> 8 );

    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);

    for (std::vector<String>::iterator it = fInjectedScripts.begin(); it != fInjectedScripts.end(); ++it) {
        injectScript(*it);
    }

    navigate(fUrl);

    fInjectedScripts.clear();

    return S_OK;
}

HRESULT EdgeWebView::handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                                       ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;

    handleLoadFinished();
    
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                      ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    // Edge WebView2 does not provide access to JSCore values; resort to parsing JSON
    (void)sender;

    LPWSTR jsonStr;
    ICoreWebView2WebMessageReceivedEventArgs_get_WebMessageAsJson(eventArgs, &jsonStr);
    cJSON* jArgs = cJSON_Parse(TO_LPCSTR(jsonStr));
    CoTaskMemFree(jsonStr);

    JsValueVector args;
    
    if (cJSON_IsArray(jArgs)) {
        int numArgs = cJSON_GetArraySize(jArgs);

        if (numArgs > 0) {
            for (int i = 0; i < numArgs; i++) {
                cJSON* jArg = cJSON_GetArrayItem(jArgs, i);

                if (cJSON_IsFalse(jArg)) {
                    args.push_back(JsValue(false));
                } else if (cJSON_IsTrue(jArg)) {
                    args.push_back(JsValue(true));
                } else if (cJSON_IsNumber(jArg)) {
                    args.push_back(JsValue(cJSON_GetNumberValue(jArg)));
                } else if (cJSON_IsString(jArg)) {
                    args.push_back(JsValue(String(cJSON_GetStringValue(jArg))));
                } else {
                    args.push_back(JsValue()); // null
                }
            }
        }
    }

    cJSON_free(jArgs);

    handleScriptMessage(args);
    
    return S_OK;
}

void EdgeWebView::webViewLoaderErrorMessageBox(HRESULT result)
{
    std::wstringstream wss;
    wss << "The Microsoft Edge WebView2 Runtime could not be initialized.\n"
        << "Code 0x" << std::hex << result
        << "\n\n"
        << "Make sure it is installed on your system. Clicking OK will open its"
        << " download page. Once installed close and reopen the plugin window."
        << "\n\n";

    std::wstring ws = wss.str();

    int id = MessageBox(0, ws.c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OKCANCEL | MB_ICONEXCLAMATION);

    if (id == IDOK) {
        ShellExecute(0, L"open", L"" WEBVIEW2_DOWNLOAD_URL, 0, 0, SW_SHOWNORMAL);
    }
}
