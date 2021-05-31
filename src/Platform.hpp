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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "extra/String.hpp"

START_NAMESPACE_DISTRHO

namespace platform {

    // The following functions allow to locate resource files and helper binaries
    // during runtime. Both are relative to the running binary path. 

    String getTemporaryPath();
    String getExecutablePath();
    String getSharedLibraryPath();
    String getBinaryPath();
    String getBinaryDirectoryPath();
    String getResourcePath();

    // Helps scaling web views on high density displays

    float getSystemDisplayScaleFactor();

}

END_NAMESPACE_DISTRHO

#endif  // PLATFORM_HPP
