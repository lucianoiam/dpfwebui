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

#ifndef WEBVIEW2ENVIRONMENTOPTIONS_HPP
#define WEBVIEW2ENVIRONMENTOPTIONS_HPP

#define UNICODE
#define CINTERFACE

#include "WebView2.h"

#define VALUE_MAX 256

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
