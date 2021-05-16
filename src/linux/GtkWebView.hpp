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

#ifndef GTKWEBVIEW_HPP
#define GTKWEBVIEW_HPP

#include <sys/types.h>

#include "../WebView.hpp"
#include "DistrhoDefines.h"

START_NAMESPACE_DISTRHO

class GtkWebView : public WebView
{
public:
    GtkWebView();
    ~GtkWebView();
    
    void reparent(uintptr_t parentWindowId);

private:
    void cleanup();
    
	pid_t fView;	// naming it view for coherence, the helper is the view

};

END_NAMESPACE_DISTRHO

#endif  // GTKWEBVIEW_HPP
