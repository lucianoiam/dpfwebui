/*
 * dpf-webui
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

#include <stdio.h>

#include "comutil.h"

LPWSTR refiid2wstr(LPWSTR buf, REFIID riid)
{
    swprintf(buf, 37, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        riid->Data1,
        riid->Data2,
        riid->Data3,
        riid->Data4[0],
        riid->Data4[1],
        riid->Data4[2],
        riid->Data4[3],
        riid->Data4[4],
        riid->Data4[5],
        riid->Data4[6],
        riid->Data4[7]
    );
    return buf;
}
