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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "extra/String.hpp"

#include "macro.h"

START_NAMESPACE_DISTRHO

namespace platform {

    // The following functions help locating resource files and helper binaries
    // during runtime. Location of such is relative to the running binary path. 

    String getBinaryDirectoryPath();
    String getBinaryPath();
    String getSharedLibraryPath();
    String getExecutablePath();
    String getResourcePath();
    String getTemporaryPath();

    // Helps scaling the web view on high density displays

    float getSystemDisplayScaleFactor();

    const String kDefaultResourcesSubdirectory = String(XSTR(BIN_BASENAME) "_resources");

}

END_NAMESPACE_DISTRHO

#endif  // PLATFORM_HPP
