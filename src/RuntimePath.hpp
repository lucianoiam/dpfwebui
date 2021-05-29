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

#ifndef RUNTIME_PATH_HPP
#define RUNTIME_PATH_HPP

#include "extra/String.hpp"

// These functions' main goal is to determine the location of web resource files
// on disk during runtime. Resources location is relative to the binary location.

START_NAMESPACE_DISTRHO

namespace rtpath {

    String getTemporaryPath();
    String getExecutablePath();
    String getSharedLibraryPath();
    String getBinaryPath();
    String getBinaryDirectoryPath();
    String getResourcePath();

}

END_NAMESPACE_DISTRHO

#endif  // RUNTIME_PATH_HPP
