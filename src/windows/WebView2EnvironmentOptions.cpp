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

#include <cstdio>
#include <guiddef.h>

#include "com.h"

using namespace edge;

#define pInstance static_cast<WebView2EnvironmentOptions*>(This)

static HRESULT STDMETHODCALLTYPE QueryInterface(ICoreWebView2EnvironmentOptions* This, REFIID riid, void** ppvObject)
{
    WCHAR temp[37];
    refiid2wstr(temp, riid);

    ::wprintf(L"Query %ls : ", temp);

    if (!::IsEqualIID(riid, IID_ICoreWebView2EnvironmentOptions)) {
        *ppvObject = 0;
        ::wprintf(L"E_NOINTERFACE\n");
        return E_NOINTERFACE;
    }

    *ppvObject = This;

    ::wprintf(L"NOERROR\n");
    return NOERROR;
}

static ULONG STDMETHODCALLTYPE Null_AddRef(ICoreWebView2EnvironmentOptions* This)
{
    (void)This;
    return 1;
}

static ULONG STDMETHODCALLTYPE Null_Release(ICoreWebView2EnvironmentOptions* This)
{
    (void)This;
    return 1;
}

static HRESULT STDMETHODCALLTYPE Impl_get_AdditionalBrowserArguments(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    *value = pInstance->fAdditionalBrowserArguments;
    ::wprintf(L"get %ls\n", pInstance->fAdditionalBrowserArguments);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_AdditionalBrowserArguments(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    wcscpy(pInstance->fAdditionalBrowserArguments, value);
    ::wprintf(L"put %ls\n", pInstance->fAdditionalBrowserArguments);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_Language(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    *value = pInstance->fLanguage;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_Language(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    wcscpy(pInstance->fLanguage, value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_TargetCompatibleBrowserVersion(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    *value = pInstance->fTargetCompatibleBrowserVersion;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_TargetCompatibleBrowserVersion(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    wcscpy(pInstance->fTargetCompatibleBrowserVersion, value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_AllowSingleSignOnUsingOSPrimaryAccount(
    ICoreWebView2EnvironmentOptions * This, BOOL *allow)
{
    *allow = pInstance->fAllowSingleSignOnUsingOSPrimaryAccount;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_AllowSingleSignOnUsingOSPrimaryAccount(
    ICoreWebView2EnvironmentOptions * This, BOOL allow)
{
    pInstance->fAllowSingleSignOnUsingOSPrimaryAccount = allow;
    return S_OK;
}

static ICoreWebView2EnvironmentOptionsVtbl Vtbl_WebView2EnvironmentOptions = {
    QueryInterface,
    Null_AddRef,
    Null_Release,
    Impl_get_AdditionalBrowserArguments,
    Impl_put_AdditionalBrowserArguments,
    Impl_get_Language,
    Impl_put_Language,
    Impl_get_TargetCompatibleBrowserVersion,
    Impl_put_TargetCompatibleBrowserVersion,
    Impl_get_AllowSingleSignOnUsingOSPrimaryAccount,
    Impl_put_AllowSingleSignOnUsingOSPrimaryAccount
};

WebView2EnvironmentOptions::WebView2EnvironmentOptions()
    : ICoreWebView2EnvironmentOptions { &Vtbl_WebView2EnvironmentOptions }
{
    fAdditionalBrowserArguments[0] = '\0';
    fLanguage[0] = '\0';
    fTargetCompatibleBrowserVersion[0] = '\0';
    fAllowSingleSignOnUsingOSPrimaryAccount = FALSE;
}
