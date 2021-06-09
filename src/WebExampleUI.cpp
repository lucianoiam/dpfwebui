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

#include "WebExampleUI.hpp"

// These dimensions are scaled up according to the system display scaling
// configuration on all platforms except Mac where it does not apply.
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
    // Web view is not guaranteed to be ready yet. Calls to webView().runScript()
    // or any mapped WebUI methods are forbidden. Mapped methods are those that
    // have their JavaScript counterparts; they rely on message passing and
    // ultimately webView().runScript(). Still can call webView().injectScript()
    // to queue scripts that will run immediately after content finishes loading
    // and before any referenced scripts (<script src="...">) start running.
}

void WebExampleUI::webViewLoadFinished()
{
    // Called when the main document finished loading and DOM is ready. It is now
    // safe to call runScript() if needed.
}

bool WebExampleUI::webViewScriptMessageReceived(const ScriptValueVector& args)
{
    // DOM is guaranteed to be ready here. Can override parent class behavior.
    return WebUI::webViewScriptMessageReceived(args);
}
