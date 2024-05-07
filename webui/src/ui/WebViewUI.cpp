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

#include <cstring>

#include "WebViewUI.hpp"

#include "extra/CSSColor.hpp"
#include "extra/Path.hpp"

#define HTML_INDEX_PATH "/ui/index.html"

USE_NAMESPACE_DISTRHO

static const uintptr_t kOriginEmbeddedWebView = 0;

WebViewUI::WebViewUI(uint widthCssPx, uint heightCssPx, const char* backgroundCssColor,
                     float initPixelRatio)
    : WebViewUIBase(widthCssPx, heightCssPx, initPixelRatio)
    , fBackgroundColor(CSSColor::fromHex(backgroundCssColor))
    , fJsUiReady(false)
    , fPlatformWindow(0)
    , fWebView(nullptr)
#if defined(DPF_WEBUI_NETWORK_UI)
    , fNavigated(false)
#endif
{
    setBuiltInFunctionHandlers();
}

WebViewUI::~WebViewUI()
{
    if (fWebView != nullptr) {
        fWebView->setEventHandler(nullptr);
        delete fWebView;
    }
}

void WebViewUI::setWebView(WebViewBase* webView)
{
    fWebView = webView;

    fWebView->setEventHandler(this);
#if defined(DPF_WEBUI_PRINT_TRAFFIC)
    fWebView->setPrintTraffic(true);
#endif
    
#if defined(DPF_WEBUI_INJECT_FRAMEWORK_JS) && !defined(DPF_WEBUI_NETWORK_UI) 
    String js = String(
#include "ui/dpf.js.inc"
    );
    fWebView->injectScript(js);
#endif

    fPlatformWindow = isStandalone() ? createStandaloneWindow() : getParentWindowHandle();
    fWebView->setParent(fPlatformWindow);
    fWebView->setBackgroundColor(fBackgroundColor);

    // Convert CSS pixels to native following the final web view pixel ratio.
    // Then adjust window size so it correctly wraps web content on high density
    // displays, known as Retina or HiDPI. WebViewBase::getDevicePixelRatio()
    // needs a parent window to be set because scaling can vary across displays.
    const float k = fWebView->getDevicePixelRatio();
    const uint width = static_cast<uint>(k * static_cast<float>(getInitWidthCSS()));
    const uint height = static_cast<uint>(k * static_cast<float>(getInitHeightCSS()));
    fWebView->setSize(width, height);
    fWebView->realize();

    setSize(width, height);
}

void WebViewUI::load()
{
    if (fWebView != nullptr) {
#if defined(DPF_WEBUI_NETWORK_UI) 
        if ((! DISTRHO_PLUGIN_WANT_STATE) || isStandalone()) {
            // State is needed for reusing web server port
            String url = getLocalUrl();
            fWebView->navigate(url);
        }
#else
        String url = "file://" + Path::getPluginLibrary() + HTML_INDEX_PATH;
        fWebView->navigate(url);
#endif
    }
}

void WebViewUI::runScript(String& source)
{
    if (fWebView != nullptr) {
        fWebView->runScript(source);
    }
}

void WebViewUI::injectScript(String& source)
{
    // Cannot inject scripts after navigation has started
    if (fWebView != nullptr) {
        fWebView->injectScript(source);
    }
}

void WebViewUI::ready()
{
    fJsUiReady = true;

    for (MessageBuffer::iterator it = fMessageBuffer.begin(); it != fMessageBuffer.end(); ++it) {
        fWebView->postMessage(*it);
    }
    
    fMessageBuffer.clear();
}

void WebViewUI::setKeyboardFocus(bool focus)
{
    fWebView->setKeyboardFocus(focus);
}

#if ! defined(DPF_WEBUI_NETWORK_UI)
void WebViewUI::postMessage(const Variant& payload, uintptr_t /*destination*/, uintptr_t /*exclude*/)
{
    if (fJsUiReady) {
        fWebView->postMessage(payload);
    } else {
        fMessageBuffer.push_back(payload);
    }
}
#endif

void WebViewUI::uiIdle()
{
    WebViewUIBase::uiIdle();

    if (isStandalone()) {
        processStandaloneEvents();
    }
}

#if DISTRHO_PLUGIN_WANT_STATE
void WebViewUI::stateChanged(const char* key, const char* value)
{
    WebViewUIBase::stateChanged(key, value);

# if defined(DPF_WEBUI_NETWORK_UI)
    if ((std::strcmp(key, "_ws_port") == 0) && (fWebView != nullptr)
            && ! fNavigated) {
        fNavigated = true;
        String url = getLocalUrl();
        fWebView->navigate(url);
    }
# endif
}
#endif

void WebViewUI::sizeChanged(uint width, uint height)
{
    WebViewUIBase::sizeChanged(width, height);
    
    queue([this, width, height] {
        fWebView->setSize(width, height);
        callback("sizeChanged", { width, height });
    });
}

void WebViewUI::setBuiltInFunctionHandlers()
{
    // These handlers only make sense for the plugin embedded web view

    setFunctionHandler("getWidth", 0, [this](const Variant&, uintptr_t origin) {
        callback("getWidth", { static_cast<double>(getWidth()) }, origin);
    });

    setFunctionHandler("getHeight", 0, [this](const Variant&, uintptr_t origin) {
        callback("getHeight", { static_cast<double>(getHeight()) }, origin);
    });

    setFunctionHandler("isResizable", 0, [this](const Variant&, uintptr_t origin) {
        callback("isResizable", { isResizable() }, origin);
    });

    setFunctionHandler("setWidth", 1, [this](const Variant& args, uintptr_t) {
        setWidth(static_cast<uint>(args[0].getNumber()));
    });

    setFunctionHandler("setHeight", 1, [this](const Variant& args, uintptr_t) {
        setHeight(static_cast<uint>(args[0].getNumber()));
    });

    setFunctionHandler("setSize", 2, [this](const Variant& args, uintptr_t) {
        setSize(
            static_cast<uint>(args[0].getNumber()), // width
            static_cast<uint>(args[1].getNumber())  // height
        );
    });

    setFunctionHandler("setKeyboardFocus", 1, [this](const Variant& args, uintptr_t) {
        setKeyboardFocus(static_cast<bool>(args[0].getBoolean()));
    });

    setFunctionHandler("ready", 0, [this](const Variant&, uintptr_t) {
        ready();
    });

    setFunctionHandler("openSystemWebBrowser", 1, [this](const Variant& args, uintptr_t) {
        String url = args[0].getString();
        openSystemWebBrowser(url);
    });
}

void WebViewUI::handleWebViewLoadFinished()
{
    onDocumentReady();
}

void WebViewUI::handleWebViewScriptMessage(const Variant& payload)
{
    handleMessage(payload, kOriginEmbeddedWebView);
}

void WebViewUI::handleWebViewConsole(const String& tag, const String& text)
{
    if (tag == "log") {
        d_stderr("%s", text.buffer());
    } else if (tag == "info") {
        d_stderr("INFO : %s", text.buffer());
    } else if (tag == "warn") {
        d_stderr("WARN : %s", text.buffer());
    } else if (tag == "error") {
        d_stderr("ERROR : %s", text.buffer());
    }
}
