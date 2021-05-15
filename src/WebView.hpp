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

#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include <cstdint>
#include <string>

#include "DistrhoDefines.h"

START_NAMESPACE_DISTRHO

class WebView
{
public:
    virtual ~WebView() {};
    
    virtual void reparent(uintptr_t parentWindowId) = 0;

protected:
    std::string getContentUrl()
    {
        // TODO
        return "https://distrho.sourceforge.io/images/screenshots/distrho-kars.png";
    }

};

END_NAMESPACE_DISTRHO

#endif  // WEBVIEW_HPP
