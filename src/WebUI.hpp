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

class WebUI : public UI, public WebViewScriptMessageHandler
{
public:
    WebUI();
    ~WebUI() {};

    void onDisplay() override;

    void parameterChanged(uint32_t index, float value) override;

    void handleWebViewScriptMessage(const ScriptMessageArguments& args) override;

protected:
    void onResize(const ResizeEvent& ev) override;

private:
    WEBVIEW_CLASS fWebView;
    uintptr_t     fParentWindowId;

};

END_NAMESPACE_DISTRHO

#endif  // WEBUI_HPP
