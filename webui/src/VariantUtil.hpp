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

#ifndef VARIANT_UTIL_HPP
#define VARIANT_UTIL_HPP

#include <vector>

#include "src/DistrhoDefines.h"

START_NAMESPACE_DISTRHO

typedef std::vector<uint8_t> BinaryData;

template<class T>
T sliceVariantArray(const T& a, int start, int end = -1) noexcept
{
    if (! a.isArray()) {
        return T();
    }

    T b = T::createArray();

    if (! a.isArray()) {
        return b;
    }

    if ((start < 0) || (start == end)) {
        return b;
    }

    const int size = a.getArraySize();

    if (start >= size) {
        return b;
    }

    if ((end < 0)/*def value*/ || (end > size)) {
        end = size;
    }

    for (int i = start; i < end; ++i) {
        b.pushArrayItem(a.getArrayItem(i));
    }

    return b;
}

template<class T>
T& joinVariantArrays(T& a, const T& b) noexcept
{
    if (! a.isArray() || ! b.isArray()) {
        return a;
    }

    const int size = b.getArraySize();

    for (int i = 0; i < size; ++i) {
        a.pushArrayItem(b.getArrayItem(i));
    }

    return a;
}

END_NAMESPACE_DISTRHO

#endif // VARIANT_UTIL_HPP
