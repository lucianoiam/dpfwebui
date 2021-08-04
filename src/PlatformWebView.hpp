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

#ifndef PLATFORM_WEB_VIEW_HPP
#define PLATFORM_WEB_VIEW_HPP

// Web view headers are included in this separate file outside Platform.hpp
// to avoid bringing UI dependencies into the Plugin.

#ifdef DISTRHO_OS_LINUX
#include "linux/ExternalGtkWebView.hpp"

namespace platform {
    typedef ExternalGtkWebView WebView;
}

#endif

#ifdef DISTRHO_OS_MAC
#include "macos/CocoaWebView.hpp"

namespace platform {
    typedef CocoaWebView WebView;
}

#endif

#ifdef DISTRHO_OS_WINDOWS
#include "windows/EdgeWebView.hpp"

namespace platform {
    typedef EdgeWebView WebView;
}

#endif

#endif  // PLATFORM_WEB_VIEW_HPP
