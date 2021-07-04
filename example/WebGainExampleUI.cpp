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
#define INIT_BACKGROUND_RGBA 0xB6EFD4FF

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebGainExampleUI;
}

WebGainExampleUI::WebGainExampleUI()
    : ProxyWebUI(BASE_WIDTH_PX, BASE_HEIGHT_PX, INIT_BACKGROUND_RGBA)
{
    // Web view not guaranteed to be ready yet. Calls to webWidget().runScript()
    // or any DPF methods mapped by ProxyWebUI are forbidden. Mapped methods are
    // those that have their JavaScript counterparts; they rely on message
    // passing and ultimately webWidget().runScript(). Still it is safe to call
    // webWidget().injectScript() to queue scripts that will run immediately
    // after web content finishes loading and before any referenced <script> runs.
}

void WebGainExampleUI::webContentReady()
{
    // Called when the main document finished loading and DOM is ready. It is
    // now safe to call webWidget().runScript() and mapped DPF methods if needed.
}

void WebGainExampleUI::webMessageReceived(const ScriptValueVector& args)
{
    // Web view and DOM are guaranteed to be ready here.
    (void)args;
}
