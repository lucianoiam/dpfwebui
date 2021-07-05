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

#ifdef DISTRHO_OS_LINUX
#include "linux/ExternalGtkWebWidget.hpp"
typedef ExternalGtkWebWidget _WebWidget;
#endif
#ifdef DISTRHO_OS_MAC
#include "macos/CocoaWebWidget.hpp"
typedef CocoaWebWidget _WebWidget;
#endif
#ifdef DISTRHO_OS_WINDOWS
#include "windows/EdgeWebWidget.hpp"
typedef EdgeWebWidget _WebWidget;
#endif

START_NAMESPACE_DISTRHO

namespace platform {

	typedef _WebWidget WebWidget;

    bool   isRunningStandalone();
    void   setRunningStandalone(bool standalone);

    float  getSystemDisplayScaleFactor();

    String getBinaryPath();
    String getResourcePath();
    String getTemporaryPath();

    const String kDefaultResourcesSubdirectory = String(XSTR(BIN_BASENAME) "_res");

}

END_NAMESPACE_DISTRHO

#endif  // PLATFORM_HPP
