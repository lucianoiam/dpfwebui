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


// WIP

WCHAR args[100];

static HRESULT STDMETHODCALLTYPE CoreWebView2EnvironmentOptions_get_AdditionalBrowserArguments(
    ICoreWebView2EnvironmentOptions * This, LPWSTR *value)
{
    wcscpy(args, L"");
    *value = args;
    return S_OK;
}

static ICoreWebView2EnvironmentOptionsVtbl CoreWebView2EnvironmentOptionsVtbl =
{
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    CoreWebView2EnvironmentOptions_get_AdditionalBrowserArguments
    // ...
};

class CoreWebView2EnvironmentOptions: public ICoreWebView2EnvironmentOptions
{
public:
    CoreWebView2EnvironmentOptions()
        : ICoreWebView2EnvironmentOptions {&CoreWebView2EnvironmentOptionsVtbl}
    {

    }

};
