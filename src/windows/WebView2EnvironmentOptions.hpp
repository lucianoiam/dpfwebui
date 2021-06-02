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

#ifndef WEBVIEW2ENVIRONMENTOPTIONS_HPP
#define WEBVIEW2ENVIRONMENTOPTIONS_HPP

#define UNICODE
#define CINTERFACE

#include "WebView2.h"

#define VALUE_MAX 1024

namespace edge {

class WebView2EnvironmentOptions : public ICoreWebView2EnvironmentOptions
{
public:
    WebView2EnvironmentOptions();
    ~WebView2EnvironmentOptions() {};

    WCHAR fAdditionalBrowserArguments[VALUE_MAX];
    WCHAR fLanguage[VALUE_MAX];
    WCHAR fTargetCompatibleBrowserVersion[VALUE_MAX];
    BOOL  fAllowSingleSignOnUsingOSPrimaryAccount; 
};

} // namespace edge

#endif // WEBVIEW2ENVIRONMENTOPTIONS_HPP
