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

#ifndef EDGEWEBVIEWUI_HPP
#define EDGEWEBVIEWUI_HPP

#define UNICODE

#include <string>

#include "DistrhoDefines.h"

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
    ~EdgeWebViewUI();
    
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
