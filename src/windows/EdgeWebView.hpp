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
