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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "extra/String.hpp"

#include "macro.h"

#ifdef DISTRHO_OS_LINUX
#include "linux/ExternalGtkWebView.hpp"
typedef ExternalGtkWebView _WebView;
#endif
#ifdef DISTRHO_OS_MAC
#include "macos/CocoaWebView.hpp"
typedef CocoaWebView _WebView;
#endif
#ifdef DISTRHO_OS_WINDOWS
#include "windows/EdgeWebView.hpp"
typedef EdgeWebView _WebView;
#endif

START_NAMESPACE_DISTRHO

namespace platform {

    typedef _WebView WebView;

    bool   isRunningStandalone();
    void   setRunningStandalone(bool standalone);

    float  getSystemDisplayScaleFactor();

    String getBinaryPath();
    String getLibraryPath();
    String getTemporaryPath();

    const String kDefaultLibrarySubdirectory = String(XSTR(BIN_BASENAME) "_lib");

}

END_NAMESPACE_DISTRHO

#endif  // PLATFORM_HPP
