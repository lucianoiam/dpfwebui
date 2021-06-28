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

#ifndef KEYBOARDFORWARDING_HPP
#define KEYBOARDFORWARDING_HPP

#include <WinDef.h>
#include <winuser.h>

#include "src/DistrhoDefines.h"

START_NAMESPACE_DISTRHO

class KeyboardForwarding
{
    public:
        static KeyboardForwarding& getInstance()
        {
            static KeyboardForwarding instance;
            return instance;
        }

        KeyboardForwarding(const KeyboardForwarding&) = delete;
        void operator=(const KeyboardForwarding&) = delete;
        
        void incRefCount();
        void decRefCount();

        void handleLowLevelKeyEvent(HWND hPluginRootWnd, UINT message, KBDLLHOOKSTRUCT* lpData);
        void routeKeyMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        KeyboardForwarding();

        // Win32 callbacks

        static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
        static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
        static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

        int   fRefCount;
        HWND  fHostHWnd;
        HHOOK fKeyboardHook;

};

END_NAMESPACE_DISTRHO

#endif  // KEYBOARDFORWARDING_HPP
