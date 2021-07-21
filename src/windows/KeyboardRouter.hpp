/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#ifndef KEYBOARDROUTER_HPP
#define KEYBOARDROUTER_HPP

#include <WinDef.h>
#include <winuser.h>

#include "src/DistrhoDefines.h"

START_NAMESPACE_DISTRHO

class KeyboardRouter
{
    public:
        static KeyboardRouter& getInstance()
        {
            static KeyboardRouter instance;
            return instance;
        }

        KeyboardRouter(const KeyboardRouter&) = delete;
        void operator=(const KeyboardRouter&) = delete;
        
        void incRefCount();
        void decRefCount();

        void hostSendLowLevelKeyEvent(UINT message, KBDLLHOOKSTRUCT* lpData);

    private:
        KeyboardRouter();

        // Win32 callbacks

        static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);
        static BOOL CALLBACK enumChildProc(HWND hWnd, LPARAM lParam);
        static LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

        int   fRefCount;
        HWND  fHostHWnd;
        HHOOK fKeyboardHook;

};

END_NAMESPACE_DISTRHO

#endif  // KEYBOARDROUTER_HPP
