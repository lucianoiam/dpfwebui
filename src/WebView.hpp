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

#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include <cstdint>

#include "DistrhoDefines.h"
#include "extra/String.hpp"

START_NAMESPACE_DISTRHO

class WebView
{
public:
    virtual ~WebView() {};
    
    virtual void reparent(uintptr_t parentWindowId) = 0;

protected:
    String getContentUrl()
    {
        // TODO
        return String("https://distrho.sourceforge.io/images/screenshots/distrho-kars.png");
    }

};

END_NAMESPACE_DISTRHO

#endif  // WEBVIEW_HPP
