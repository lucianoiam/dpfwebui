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

#include "GtkWebView.hpp"

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

USE_NAMESPACE_DISTRHO

GtkWebView::GtkWebView()
{
    // Do not start subprocess here, some hosts like REAPER can trigger repeated
    // creation-destruction cycles of this class before settling.
}

GtkWebView::~GtkWebView()
{
    fView.terminate();
}

void GtkWebView::reparent(uintptr_t parentWindowId)
{
    if (!fView.isRunning()) {
        fView.spawn();
        fView.navigate(getContentUrl());
    }

    fView.reparent(parentWindowId);
}
