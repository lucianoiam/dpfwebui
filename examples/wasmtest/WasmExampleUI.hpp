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

#ifndef WASMEXAMPLEUI_HPP
#define WASMEXAMPLEUI_HPP

#include "WebHostUI.hpp"

START_NAMESPACE_DISTRHO

class WasmExampleUI : public WebHostUI
{
public:
    WasmExampleUI() : WebHostUI(600 /* baseWidth */, 300 /* baseHeight */,
                                0xD4B6EFFF /* backgroundColor */) {};
    ~WasmExampleUI() {};

};

END_NAMESPACE_DISTRHO

#endif  // WASMEXAMPLEUI_HPP
