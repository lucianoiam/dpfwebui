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

#include <cstring>
#include <spawn.h>
#include <signal.h> // TODO: needed for kill(), remove after IPC implemented

/*
  Need to launch a separate process hosting the GTK web view because linking
  plugins to UI toolkit libraries like GTK or QT is known to be problematic.
*/

USE_NAMESPACE_DISTRHO

extern char **environ;

GtkWebView::GtkWebView()
    : fView(0)
{
    // empty
}

GtkWebView::~GtkWebView()
{
    cleanup();
}

void GtkWebView::reparent(uintptr_t parentWindowId)
{
    // TODO: In contrast to Windows WebView2, the GTK web view can be reparented at any time thanks
    //       to XEMBED. But that first needs the plugin->helper IPC channel to be implemented.
    //       So for now the helper process is launched on every reparent() call.

    cleanup();

    char xid[sizeof(uintptr_t) + /* 0x + \0 */ 3];
    sprintf(xid, "%lx", (long)parentWindowId);

    char url[1024];
    strncpy(url, getContentUrl().c_str(), sizeof(url) - 1);

    const char *argv[] = {"helper", xid, url, NULL};
    const char* fixmeHardcodedPath = "/home/user/src/dpf-webui/bin/d_dpf_webui_helper";
    
    int status = posix_spawn(&fView, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);

    if (status != 0) {
        // TO DO
    }
}

void GtkWebView::cleanup()
{
    // TODO: send a message instead of applying brute force
    if (fView != 0) {
        kill(fView, SIGKILL);
        fView = 0;
    }
}
