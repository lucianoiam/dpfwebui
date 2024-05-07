/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#include "WindowsWebViewUI.hpp"

#include <shellapi.h>

BOOL CALLBACK FindHostWindowProc(HWND hWnd, LPARAM lParam);

USE_NAMESPACE_DISTRHO

WindowsWebViewUI::WindowsWebViewUI(uint widthCssPx, uint heightCssPx,
        const char* backgroundCssColor, bool startLoading)
    : WebViewUI(widthCssPx, heightCssPx, backgroundCssColor, EdgeWebView::getMonitorScaleFactor(0))
    , fHostHWnd(0)
{
    if (isDryRun()) {
        return;
    }

    // Web view looks blurry on Live 11 unless Auto-Scale Plugin Window feature
    // is disabled (right-click plugin). Cannot control this programmatically.
    // https://forum.juce.com/t/blurry-ui-running-vst-in-ableton-live-10/42472/5
    
    EdgeWebView* view = new EdgeWebView(String(kWebViewUserAgent));

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

    if (startLoading) {
        load();
    }
}

WindowsWebViewUI::~WindowsWebViewUI()
{
    // TODO - standalone support
}

void WindowsWebViewUI::openSystemWebBrowser(String& url)
{
    ShellExecuteA(0, "open", url.buffer(), 0, 0, SW_SHOWNORMAL);
}

uintptr_t WindowsWebViewUI::createStandaloneWindow()
{
    // TODO - standalone support
    return 0;
}

void WindowsWebViewUI::processStandaloneEvents()
{
    // TODO - standalone support
}

void WindowsWebViewUI::hostWindowSendKeyEvent(UINT message, KBDLLHOOKSTRUCT* lpData)
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
        GetWindowTextA(hWnd, (LPSTR)text, sizeof(text));

        if (strstr(text, "Ableton Live") != 0) {
            *((HWND *)lParam) = hWnd;
            return FALSE;
        }
    }

    return TRUE;
}
