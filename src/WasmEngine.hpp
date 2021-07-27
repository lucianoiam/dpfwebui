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

#ifndef WASMENGINE_HPP
#define WASMENGINE_HPP

#include <unordered_map>
#include <string>

#define WASM_API_EXTERN // link to static lib on win32
#include "wasm.h"
#include "wasmer.h"

#include "src/DistrhoDefines.h"

START_NAMESPACE_DISTRHO

class WasmEngine
{

};

END_NAMESPACE_DISTRHO

#endif  // WASMENGINE_HPP
