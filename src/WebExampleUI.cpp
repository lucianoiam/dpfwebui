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

#include "WebExampleUI.hpp"

// These dimensions are scaled up according to the system display scale factor
#define INIT_WIDTH_PX  600
#define INIT_HEIGHT_PX 300

// Color for painting the window background before the web content is ready.
// Matching it to <html> background color ensures a smooth transition.
#define INIT_BACKGROUND_RGBA 0xB6EFD4FF

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebExampleUI;
}

WebExampleUI::WebExampleUI()
    : WebUI(INIT_WIDTH_PX, INIT_HEIGHT_PX, INIT_BACKGROUND_RGBA)
{
    // Web view not guaranteed to be ready yet. Calls to webView().runScript()
    // or any DPF methods mapped by WebUI are forbidden. Mapped methods are
    // those that have their JavaScript counterparts; they rely on message
    // passing and ultimately webView().runScript(). Still it is safe to call
    // webView().injectScript() to queue scripts that will run immediately after
    // web content finishes loading and before any other <script> starts running.
}

void WebExampleUI::webContentReady()
{
    // Called when the main document finished loading and DOM is ready. It is
    // now safe to call webView().runScript() and mapped DPF methods if needed.
}

void WebExampleUI::webMessageReceived(const ScriptValueVector& args)
{
    // Web view and DOM guaranteed are to be ready here.
    (void)args;
}
