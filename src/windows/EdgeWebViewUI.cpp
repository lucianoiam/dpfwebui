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

#include "EdgeWebViewUI.hpp"

#include <codecvt>
#include <locale>
#include <sstream>
#include <shellscalingapi.h>
#include <shtypes.h>

#include "DistrhoPluginInfo.h"
#include "RuntimePath.hpp"
#include "log.h"

// https://github.com/chriskohlhoff/asio/issues/631
typedef HRESULT GETSCALEFACTORFORMONITOR(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);
typedef HRESULT GETPROCESSDPIAWARENESS(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    // GetProcessDpiAwareness() and GetScaleFactorForMonitor() are not available on Windows 7
    // Also not currently compiling on MinGW despite #include <shellscalingapi.h> (May '21)
    uint width = DISTRHO_UI_INITIAL_WIDTH;
    uint height = DISTRHO_UI_INITIAL_HEIGHT;

    HMODULE hDll = ::LoadLibrary(L"Shcore.dll");

    if (hDll) {
        GETPROCESSDPIAWARENESS *pf1 = (GETPROCESSDPIAWARENESS*)::GetProcAddress(hDll, "GetProcessDpiAwareness");
        GETSCALEFACTORFORMONITOR* pf2 = (GETSCALEFACTORFORMONITOR*)::GetProcAddress(hDll, "GetScaleFactorForMonitor");
        if (pf1 && pf2) {
            PROCESS_DPI_AWARENESS value;
            if (SUCCEEDED((*pf1)(NULL, &value))) {  // GetProcessDpiAwareness(NULL, &value)
                DEVICE_SCALE_FACTOR scale;
                HMONITOR hMon = ::MonitorFromWindow(::GetConsoleWindow(), MONITOR_DEFAULTTOPRIMARY);
                if (SUCCEEDED((*pf2)(hMon, &scale))) {  // GetScaleFactorForMonitor(hMon, &scale)
                    if (value != PROCESS_DPI_UNAWARE) {
                        width = width * (uint)scale / 100;
                        height = height * (uint)scale / 100;
                    } else {
                        // do not scale
                    }
                }
            }
        }
    }

    return new EdgeWebViewUI(width, height);
}

EdgeWebViewUI::EdgeWebViewUI(uint width, uint height)
    : WebUI(width, height)
    , fController(0)
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
    //        flicker to the point the view is unusable. Need to reinitialize everything.
    HWND hWnd = (HWND)windowId;

    cleanup();

    fHandler.EnvironmentCompleted = [this, hWnd](HRESULT result, ICoreWebView2Environment* createdEnv) {
        if (FAILED(result)) {
            errorMessageBox(L"Could not create WebView2 environment", result);
            return result;
        }

        ICoreWebView2Environment_CreateCoreWebView2Controller(createdEnv, hWnd, &fHandler);

        return S_OK;
    };

    fHandler.ControllerCompleted = [this, hWnd](HRESULT result, ICoreWebView2Controller* createdController) {
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
        color.A = 0xff; // alpha does not seem to work
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
    HRESULT result = ::GetClientRect(hWnd, &bounds);

    if (FAILED(result)) {
        LOG_STDERR_INT("Could not determine parent window size", result);
        return;
    }

    ICoreWebView2Controller2_put_Bounds(fController, bounds);
}

void EdgeWebViewUI::errorMessageBox(std::wstring message, HRESULT result)
{
    std::wstringstream ss;
    ss << message << ", HRESULT 0x" << std::hex << result;
    ::MessageBox(0, ss.str().c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONSTOP);
}
