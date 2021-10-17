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

#include <shellscalingapi.h>

#include "WindowsWebHostUI.hpp"

BOOL CALLBACK FindHostWindowProc(HWND hWnd, LPARAM lParam);

USE_NAMESPACE_DISTRHO

WindowsWebHostUI::WindowsWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
    , fHostHWnd(0)
{
    if (!shouldCreateWebView()) {
        return;
    }

    // Web view looks blurry on Live 11 unless Auto-Scale Plugin Window feature
    // is disabled (right-click plugin). Cannot control this programmatically.
    // https://forum.juce.com/t/blurry-ui-running-vst-in-ableton-live-10/42472/5
    
    EdgeWebView* view = new EdgeWebView();

    // Some hosts need key events delivered directly to their main window
    EnumWindows(FindHostWindowProc, reinterpret_cast<LPARAM>(&fHostHWnd));

    if (fHostHWnd != 0) {
        view->lowLevelKeyboardHookCallback = [this](UINT message, KBDLLHOOKSTRUCT* lpData, bool focus) {
            if (!focus) {
                hostWindowSendKeyEvent(message, lpData);
            }
        };
    }

    setWebView(view); // base class owns web view
}

WindowsWebHostUI::~WindowsWebHostUI()
{
    // TODO - standalone support
}

float WindowsWebHostUI::getDisplayScaleFactor(uintptr_t window)
{
    float k = 1.f;
    const HMODULE shcore = LoadLibrary("Shcore.dll");

    if (shcore == 0) {
        return k;
    }

    typedef HRESULT (*PFN_GetProcessDpiAwareness)(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
    typedef HRESULT (*PFN_GetScaleFactorForMonitor)(HMONITOR hMon, DEVICE_SCALE_FACTOR *pScale);

# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
    const PFN_GetProcessDpiAwareness GetProcessDpiAwareness
        = (PFN_GetProcessDpiAwareness)GetProcAddress(shcore, "GetProcessDpiAwareness");
    const PFN_GetScaleFactorForMonitor GetScaleFactorForMonitor
        = (PFN_GetScaleFactorForMonitor)GetProcAddress(shcore, "GetScaleFactorForMonitor");
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif

    PROCESS_DPI_AWARENESS dpiAware;

    if ((GetProcessDpiAwareness != 0) && (GetScaleFactorForMonitor != 0)
            && (SUCCEEDED(GetProcessDpiAwareness(0, &dpiAware)))
            && (dpiAware != PROCESS_DPI_UNAWARE)) {

        const HMONITOR hMon = MonitorFromWindow((HWND)window, MONITOR_DEFAULTTOPRIMARY);

        DEVICE_SCALE_FACTOR scaleFactor;

        if (SUCCEEDED(GetScaleFactorForMonitor(hMon, &scaleFactor))) {
            k = static_cast<float>(scaleFactor) / 100.f;
        }
    }

    FreeLibrary(shcore);

    return k;
}

void WindowsWebHostUI::openSystemWebBrowser(String& url)
{
    ShellExecute(0, "open", url.buffer(), 0, 0, SW_SHOWNORMAL);
}

uintptr_t WindowsWebHostUI::createStandaloneWindow()
{
    // TODO - standalone support
    return 0;
}

void WindowsWebHostUI::processStandaloneEvents()
{
    // TODO - standalone support
}

void WindowsWebHostUI::hostWindowSendKeyEvent(UINT message, KBDLLHOOKSTRUCT* lpData)
{
    // Translate low level keyboard events into a format suitable for SendMessage()
    WPARAM wParam = lpData->vkCode;
    LPARAM lParam = /* scan code */ lpData->scanCode << 16 | /* repeat count */ 0x1;

    switch (message) {
        case WM_KEYDOWN:
            // Basic logic that forwards a-z to allow playing with Live's virtual keyboard.
            SendMessage(fHostHWnd, WM_KEYDOWN, wParam, lParam);

            if ((lpData->vkCode >= 'A') && (lpData->vkCode <= 'Z')) {
                wParam |= 0x20; // to lowercase
                SendMessage(fHostHWnd, WM_CHAR, wParam, lParam);
            }

            break;
        case WM_KEYUP:
            // bit 30: The previous key state. The value is always 1 for a WM_KEYUP message.
            // bit 31: The transition state. The value is always 1 for a WM_KEYUP message.
            lParam |= 0xC0000000;
            SendMessage(fHostHWnd, WM_KEYUP, wParam, lParam);
            
            break;
    }
}

BOOL CALLBACK FindHostWindowProc(HWND hWnd, LPARAM lParam)
{
    DWORD winProcId = 0;
    GetWindowThreadProcessId(hWnd, &winProcId);

    if (winProcId == GetCurrentProcessId()) {
        char text[256];
        text[0] = '\0';
        GetWindowText(hWnd, (LPSTR)text, sizeof(text));

        if (strstr(text, "Ableton Live") != 0) {
            *((HWND *)lParam) = hWnd;
            return FALSE;
        }
    }

    return TRUE;
}
