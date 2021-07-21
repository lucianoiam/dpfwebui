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

#ifndef ASEXAMPLEUI_HPP
#define ASEXAMPLEUI_HPP

#include "WebHostUI.hpp"

#define BASE_WIDTH_PX  600
#define BASE_HEIGHT_PX 300
#define INIT_BACKGROUND_RGBA 0xD4B6EFFF

START_NAMESPACE_DISTRHO

class AsExampleUI : public WebHostUI
{
public:
    AsExampleUI() : WebHostUI(BASE_WIDTH_PX, BASE_HEIGHT_PX, INIT_BACKGROUND_RGBA) {}
    ~AsExampleUI() {}

};

END_NAMESPACE_DISTRHO

#endif  // ASEXAMPLEUI_HPP
