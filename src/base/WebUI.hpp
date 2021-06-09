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

#ifndef WEBUI_HPP
#define WEBUI_HPP

#include "DistrhoUI.hpp"

#ifdef DISTRHO_OS_LINUX
#include "arch/linux/ExternalGtkWebView.hpp"
#endif
#ifdef DISTRHO_OS_MAC
#include "arch/macos/CocoaWebView.hpp"
#endif
#ifdef DISTRHO_OS_WINDOWS
#include "arch/windows/EdgeWebView.hpp"
#endif

START_NAMESPACE_DISTRHO

class WebUI : public UI, private WebViewEventHandler
{
public:
    WebUI(uint width = 0, uint height = 0, uint32_t backgroundColor = 0xffffffff);
    virtual ~WebUI() {};

protected:

    void onDisplay() override;
    void onResize(const ResizeEvent& ev) override;

    void parameterChanged(uint32_t index, float value) override;
#if DISTRHO_PLUGIN_WANT_STATE
    void stateChanged(const char* key, const char* value) override;
#endif

    DISTRHO_WEBVIEW_CLASS& webView() { return fWebView; }

    void webPostMessage(const ScriptValueVector& args) { fWebView.postMessage(args); }

    virtual void webContentReady() {};
    virtual void webMessageReceived(const ScriptValueVector& args) { (void)args; };

private:
    // WebViewEventHandler

    virtual void handleWebViewLoadFinished() override;
    virtual void handleWebViewScriptMessageReceived(const ScriptValueVector& args) override;

    DISTRHO_WEBVIEW_CLASS fWebView;
    
    uint32_t fBackgroundColor;
    bool     fDisplayed;

};

END_NAMESPACE_DISTRHO

#endif  // WEBUI_HPP
