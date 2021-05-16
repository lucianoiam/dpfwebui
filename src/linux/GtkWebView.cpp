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
