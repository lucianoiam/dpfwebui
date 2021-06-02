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
    ::wprintf(L"get_AdditionalBrowserArguments %ls\n", pInstance->fAdditionalBrowserArguments);
    *value = pInstance->fAdditionalBrowserArguments;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_AdditionalBrowserArguments(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    ::wprintf(L"put_AdditionalBrowserArguments %ls\n", value);
    wcscpy(pInstance->fAdditionalBrowserArguments, value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_Language(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    ::wprintf(L"get_Language %ls\n", pInstance->fLanguage);
    *value = pInstance->fLanguage;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_Language(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    ::wprintf(L"put_Language %ls\n", value);
    wcscpy(pInstance->fLanguage, value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_TargetCompatibleBrowserVersion(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    ::wprintf(L"get_TargetCompatibleBrowserVersion %ls\n", pInstance->fTargetCompatibleBrowserVersion);
    *value = pInstance->fTargetCompatibleBrowserVersion;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_TargetCompatibleBrowserVersion(
    ICoreWebView2EnvironmentOptions * This, LPCWSTR value)
{
    ::wprintf(L"put_TargetCompatibleBrowserVersion %ls\n", value);
    wcscpy(pInstance->fTargetCompatibleBrowserVersion, value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_get_AllowSingleSignOnUsingOSPrimaryAccount(
    ICoreWebView2EnvironmentOptions * This, BOOL *allow)
{
    ::wprintf(L"get_AllowSingleSignOnUsingOSPrimaryAccount %d\n", pInstance->fAllowSingleSignOnUsingOSPrimaryAccount);
    *allow = pInstance->fAllowSingleSignOnUsingOSPrimaryAccount;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Impl_put_AllowSingleSignOnUsingOSPrimaryAccount(
    ICoreWebView2EnvironmentOptions * This, BOOL allow)
{
    ::wprintf(L"put_AllowSingleSignOnUsingOSPrimaryAccount %d\n", allow);
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
