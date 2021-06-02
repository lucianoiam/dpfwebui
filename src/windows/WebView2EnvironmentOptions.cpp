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

#include "WebView2EnvironmentOptions.hpp"
#include "com.hpp"

using namespace edge;
using namespace com;

// WIP

static HRESULT STDMETHODCALLTYPE Impl_get_AdditionalBrowserArguments(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
	// TODO
    return S_OK;
}

static ICoreWebView2EnvironmentOptionsVtbl Vtbl_WebView2EnvironmentOptions = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    Impl_get_AdditionalBrowserArguments
    // TODO ... missing args produces compilation warnings
};

WebView2EnvironmentOptions::WebView2EnvironmentOptions()
    : ICoreWebView2EnvironmentOptions {&Vtbl_WebView2EnvironmentOptions}
{}
