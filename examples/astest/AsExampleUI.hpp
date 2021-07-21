/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#ifndef ASEXAMPLEUI_HPP
#define ASEXAMPLEUI_HPP

#include "WebHostUI.hpp"

#define BASE_WIDTH_PX  600
#define BASE_HEIGHT_PX 300
#define INIT_BACKGROUND_RGBA 0xD4B6EFFF

START_NAMESPACE_DISTRHO

class AsExampleUI : public WebHostUI
{
public:
    AsExampleUI() : WebHostUI(BASE_WIDTH_PX, BASE_HEIGHT_PX, INIT_BACKGROUND_RGBA) {}
    ~AsExampleUI() {}

};

END_NAMESPACE_DISTRHO

#endif  // ASEXAMPLEUI_HPP
