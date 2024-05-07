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

#include <errno.h>

#include "LinuxWebViewUI.hpp"
#include "scaling.h"

USE_NAMESPACE_DISTRHO

LinuxWebViewUI::LinuxWebViewUI(uint widthCssPx, uint heightCssPx,
        const char* backgroundCssColor, bool startLoading)
    : WebViewUI(widthCssPx, heightCssPx, backgroundCssColor, device_pixel_ratio())
#if defined(DPF_WEBUI_NETWORK_UI) && defined(DPF_WEBUI_LINUX_WEBVIEW_CEF)
    , fFirstClient(false)
#endif
{
    if (isDryRun()) {
        return;
    }

    setWebView(new ChildProcessWebView(String(kWebViewUserAgent)));

    if (startLoading) {
        load();
    }
}

LinuxWebViewUI::~LinuxWebViewUI()
{
    // TODO - standalone support
}

void LinuxWebViewUI::openSystemWebBrowser(String& url)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "xdg-open %s", url.buffer());

    if (system(buf) != 0) {
        d_stderr("Could not open system web browser - %s", strerror(errno));
    }
}

uintptr_t LinuxWebViewUI::createStandaloneWindow()
{
    // TODO - standalone support
    return 0;
}

void LinuxWebViewUI::processStandaloneEvents()
{
    // TODO - standalone support
}

// No way for setting WebSocket request headers for the CEF web view. Force user
// agent for the first connected client instead. While this depends on timing,
// in practice the first client will be always the plugin embedded web view.
#if defined(DPF_WEBUI_NETWORK_UI) && defined(DPF_WEBUI_LINUX_WEBVIEW_CEF)
void LinuxWebViewUI::onClientConnected(Client client)
{
    if (fFirstClient) {
        return;
    }

    fFirstClient = true;

    String userAgent(kWebViewUserAgent);
    getServer().setClientUserAgent(client, userAgent);

}
#endif
