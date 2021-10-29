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

#include "WebGainExampleUI.hpp"

// These dimensions are scaled up according to the system display scale factor
#define BASE_WIDTH_PX  600
#define BASE_HEIGHT_PX 300

// Color for painting the window background before the web content is ready.
// Matching it to <html> background color ensures a smooth transition.
#define INIT_BACKGROUND_RGBA 0xD4B6EFFF

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebGainExampleUI;
}

WebGainExampleUI::WebGainExampleUI()
    : WebHostUI(BASE_WIDTH_PX, BASE_HEIGHT_PX, INIT_BACKGROUND_RGBA, false)
{
    // Web view not ready yet. Calls to runScript() or any DPF methods mapped by
    // WebHostUI are forbidden. Mapped methods are those that have their
    // counterparts in JavaScript; they rely on message passing and ultimately
    // runScript(). Setting the parent class constructor parameter startLoading
    // to false gives a chance to inject any needed scripts here, for example:

    String js = String(
        "window.testInjectedFunction = () => {"
        "   console.log(`The device pixel ratio is ${window.devicePixelRatio}`);"
        "};"
    );
    injectScript(js);

    // Injected scripts are queued to run immediately after the web content
    // finishes loading and before any referenced <script> starts executing.
    // It is not possible to inject scripts after calling load(). If
    // startLoading==false do not forget to call load() before returning:

    load();
}

void WebGainExampleUI::onWebContentReady()
{
    // Called when the main document finished loading and DOM is ready. It is
    // now safe to call runScript() and mapped DPF methods.
}

void WebGainExampleUI::onWebMessageReceived(const JsValueVector& args)
{
    // Web view and DOM are guaranteed to be ready here.
    (void)args;
}
