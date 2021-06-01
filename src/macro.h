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

#ifndef MACRO_H
#define MACRO_H

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define XSTR(s) STR(s)
#define STR(s) #s

// Actually unpacks and normalizes
#define UNPACK_RGBA(c,t) ( c >> 24)               / (t)(255), \
                         ((c & 0x00ff0000) >> 16) / (t)(255), \
                         ((c & 0x0000ff00) >> 8 ) / (t)(255), \
                         ( c & 0x000000ff)        / (t)(255)

#endif  // MACRO_H
