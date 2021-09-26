/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PATH_HPP
#define PATH_HPP

#include "distrho/extra/String.hpp"

#include "macro.h"

START_NAMESPACE_DISTRHO

namespace path {

    bool isLoadedFromSharedLibrary();
    void setLoadedFromSharedLibrary(bool loadedFromSharedLibrary);

    String getBinaryPath();
    String getLibraryPath();
    String getCachesPath();

    const String kDefaultLibrarySubdirectory = String(XSTR(PLUGIN_BIN_BASENAME) "-lib");
    const String kDefaultCacheSubdirectory = String("cache");

}

END_NAMESPACE_DISTRHO

#endif  // PATH_HPP
