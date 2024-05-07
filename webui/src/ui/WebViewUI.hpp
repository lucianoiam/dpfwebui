/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#ifndef WEBVIEW_UI_HPP
#define WEBVIEW_UI_HPP

#include <vector>

#include "WebUIBase.hpp"
#include "WebViewBase.hpp"

#if defined(DPF_WEBUI_NETWORK_UI)
# include "NetworkUI.hpp"
#endif

START_NAMESPACE_DISTRHO

#if defined(DPF_WEBUI_NETWORK_UI)
typedef NetworkUI WebViewUIBase; // https:// + WebSockets messaging
#else
typedef WebUIBase WebViewUIBase; // file://  + Native interface [postMessage()]
#endif

class WebViewUI : public WebViewUIBase, private WebViewEventHandler
{
public:
    WebViewUI(uint widthCssPx, uint heightCssPx, const char* backgroundCssColor,
              float initPixelRatio);
    virtual ~WebViewUI();

    uintptr_t getPlatformWindow() const { return fPlatformWindow; }

    WebViewBase& getWebView() { return *fWebView; }

    virtual void openSystemWebBrowser(String& url) = 0;

protected:
    void setWebView(WebViewBase* webView);

    void load();
    
    void runScript(String& source);
    void injectScript(String& source);

    void ready();
    void setKeyboardFocus(bool focus);

#if ! defined(DPF_WEBUI_NETWORK_UI)
    void postMessage(const Variant& payload, uintptr_t destination, uintptr_t exclude) override;
#endif

    void uiIdle() override;
#if DISTRHO_PLUGIN_WANT_STATE
    void stateChanged(const char* key, const char* value) override;
#endif
    void sizeChanged(uint width, uint height) override;

    virtual void onDocumentReady() {}

    virtual uintptr_t createStandaloneWindow() = 0;
    virtual void      processStandaloneEvents() = 0;

private:
    void setBuiltInFunctionHandlers();

    // WebViewEventHandler

    virtual void handleWebViewLoadFinished() override;
    virtual void handleWebViewScriptMessage(const Variant& payload) override;
    virtual void handleWebViewConsole(const String& tag, const String& text) override;

    typedef std::vector<Variant> MessageBuffer;

    uint32_t      fBackgroundColor;
    bool          fJsUiReady;
    uintptr_t     fPlatformWindow;
    WebViewBase*  fWebView;
    MessageBuffer fMessageBuffer;
#if defined(DPF_WEBUI_NETWORK_UI)
    bool          fNavigated;
#endif

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewUI)

};

END_NAMESPACE_DISTRHO

#endif  // WEBVIEW_UI_HPP
