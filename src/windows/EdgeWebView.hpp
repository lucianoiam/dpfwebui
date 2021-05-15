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

#ifndef EDGEWEBVIEW_HPP
#define EDGEWEBVIEW_HPP

#include "../WebView.hpp"
#include <string>
#include "src/DistrhoDefines.h"

/*
  The "right" way to work with WebView2 requires WIL which is provided by the
  NuGet package Microsoft.Windows.ImplementationLibrary, but WIL is not
  compatible with MinGW, see https://github.com/microsoft/wil/issues/117
  
  A working solution was copied from here:
  https://github.com/jchv/webview2-in-mingw/blob/master/Src/main.cpp

  The C++ example linked above defines CINTERFACE, ie. it calls the web view
  through a C interface, why?. Not defining CINTEFACE makes lpVtbl unavailable
*/

#define CINTERFACE
#include "WebView2.h"   // from microsoft sdk
#include "event.h"      // from example

START_NAMESPACE_DISTRHO

class EdgeWebView : public WebView
{
public:
    EdgeWebView();
    ~EdgeWebView();
    
    void reparent(uintptr_t parentWindowId);

private:
    void close();
    void resize(HWND hWnd);
    void errorMessageBox(std::string message, HRESULT result);
    void getDataPath(LPTSTR szOut, DWORD nSize);

    EventHandler             fHandler;
    ICoreWebView2Controller* fController;
    ICoreWebView2*           fView;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEW_HPP
