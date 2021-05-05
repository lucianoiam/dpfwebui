/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
 *
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

// TO DO approach:
// Linux: launch process with GTK+WebKit to avoiding host GTK issues, then reparent
// macOS: use system webview
// Windows: use system webview

#include "WebUI.hpp"
#include "Window.hpp"

#include <syslog.h>

// TODO 2: move linux specific code to separate file
#include <spawn.h>
extern char **environ;
// ------

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebUI;
}

WebUI::WebUI()
    : UI(800, 600)
{
    syslog(LOG_INFO, "%p WebUI::WebUI()", this);

    // TODO : some hosts like REAPER recreate the parent window every time
    //        the plugin UI is opened

    // UI and DSP code are completely isolated, pass opaque pointer as the owner
    uintptr_t windowId = getParentWindow().getWindowId();


    // TODO 2 - proof of concept
    char strWindowId[sizeof(uintptr_t) + /* 0x + \0 */ 3];
    sprintf(strWindowId, "%lx", (long)windowId);
    pid_t pid;
    const char *argv[] = {"helper", strWindowId, "https://distrho.sourceforge.io/images/screenshots/distrho-kars.png", NULL};
    const char* fixmeHardcodedPath = "/home/user/src/dpf-webui/bin/helper";
    int status = posix_spawn(&pid, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);
    syslog(LOG_INFO, "posix_spawn() status %d\n", status);

}

WebUI::~WebUI()
{
    syslog(LOG_INFO, "%p WebUI::~WebUI()", this);
}

void WebUI::onDisplay()
{
    syslog(LOG_INFO, "%p WebUI::onDisplay()", this);
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}
