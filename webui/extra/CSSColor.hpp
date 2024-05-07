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

#ifndef CSS_COLOR_HPP
#define CSS_COLOR_HPP

#include <cstdlib>
#include <cstring>

#include "src/DistrhoDefines.h"

START_NAMESPACE_DISTRHO

struct CSSColor
{
    // Modified version of DGL Color::fromHTML() that adds alpha support.
    // Original source in dpf/dgl/Color.hpp 
    static uint32_t fromHex(const char* rgba) noexcept
    {
        uint32_t fallback = 0xff; // solid black
        DISTRHO_SAFE_ASSERT_RETURN(rgba != nullptr && rgba[0] != '\0', fallback);

        if (rgba[0] == '#')
            ++rgba;
        DISTRHO_SAFE_ASSERT_RETURN(rgba[0] != '\0', fallback);

        std::size_t rgblen = std::strlen(rgba);
        DISTRHO_SAFE_ASSERT_RETURN(rgblen == 3 || rgblen == 6 || rgblen == 8, fallback);

        char rgbtmp[5] = { '0', 'x', '\0', '\0', '\0' };
        uint32_t r, g, b, a;

        if (rgblen == 3)
        {
            rgbtmp[2] = rgba[0];
            r = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16)) * 17;

            rgbtmp[2] = rgba[1];
            g = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16)) * 17;

            rgbtmp[2] = rgba[2];
            b = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16)) * 17;

            a = 0xff;
        }
        else
        {
            rgbtmp[2] = rgba[0];
            rgbtmp[3] = rgba[1];
            r = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16));

            rgbtmp[2] = rgba[2];
            rgbtmp[3] = rgba[3];
            g = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16));

            rgbtmp[2] = rgba[4];
            rgbtmp[3] = rgba[5];
            b = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16));

            if (rgblen == 8) {
                rgbtmp[2] = rgba[6];
                rgbtmp[3] = rgba[7];
                a = static_cast<uint32_t>(std::strtol(rgbtmp, nullptr, 16));
            } else {
                a = 0xff;
            }
        }

        return (r << 24) | (g << 16) | (b << 8) | a;
    }

};

END_NAMESPACE_DISTRHO

#endif // CSS_COLOR_HPP
