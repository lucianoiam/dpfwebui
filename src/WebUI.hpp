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

#include <cstdint>

#include "DistrhoUI.hpp"
#include "extra/String.hpp"

#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

class WebUI : public UI
{
public:
    WebUI();
    virtual ~WebUI() {};

    void onDisplay() override;

#ifdef DISTRHO_UI_BACKGROUND_COLOR
    // Hides UI method that attempts to query host
    uint getBackgroundColor() const noexcept { return DISTRHO_UI_BACKGROUND_COLOR; };
#endif

protected:
    virtual void reparent(uintptr_t parentWindowId) = 0;

    uintptr_t getParentWindowId() { return fParentWindowId; }
    
    String    getContentUrl();

private:
    uintptr_t fParentWindowId;

};

END_NAMESPACE_DISTRHO

#endif  // WEBUI_HPP
