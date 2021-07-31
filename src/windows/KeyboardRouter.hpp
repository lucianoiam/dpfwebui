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

#ifndef KEYBOARDROUTER_HPP
#define KEYBOARDROUTER_HPP

#include <WinDef.h>
#include <winuser.h>

#include "dgl/Base.hpp"

START_NAMESPACE_DGL

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

END_NAMESPACE_DGL

#endif  // KEYBOARDROUTER_HPP
