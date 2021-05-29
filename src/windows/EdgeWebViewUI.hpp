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

#ifndef EDGEWEBVIEWUI_HPP
#define EDGEWEBVIEWUI_HPP

#define UNICODE

#include <string>

/*
  The official way to work with WebView2 requires WIL which is provided by the
  NuGet package Microsoft.Windows.ImplementationLibrary, but WIL is not
  compatible with MinGW. See https://github.com/microsoft/wil/issues/117
  
  A working solution was copied from here:
  https://github.com/jchv/webview2-in-mingw/blob/master/Src/main.cpp

  The C++ example linked above defines CINTERFACE, ie. it calls the web view
  through a C interface, why?. Not defining CINTEFACE makes lpVtbl unavailable
*/

#define CINTERFACE
#include "WebView2.h"   // from microsoft sdk
#include "event.h"      // from example

#include "WebUI.hpp"

START_NAMESPACE_DISTRHO

class EdgeWebViewUI : public WebUI
{
public:
    EdgeWebViewUI();
    virtual ~EdgeWebViewUI();
    
    void parameterChanged(uint32_t index, float value) override;

    void reparent(uintptr_t windowId) override;
    
private:
    void cleanup();
    void resize(HWND hWnd);
    void errorMessageBox(std::wstring message, HRESULT result);
    std::wstring getTempPath();

    EventHandler             fHandler;
    ICoreWebView2Controller* fController;
    ICoreWebView2*           fView;

};

END_NAMESPACE_DISTRHO

#endif  // EDGEWEBVIEWUI_HPP
