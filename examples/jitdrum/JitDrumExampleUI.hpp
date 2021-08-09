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

#ifndef JITDRUM_EXAMPLE_UI_HPP
#define JITDRUM_EXAMPLE_UI_HPP

#include "WebHostUI.hpp"

#define BASE_WIDTH_PX  420
#define BASE_HEIGHT_PX 180
#define INIT_BACKGROUND_RGBA 0x202020FF

START_NAMESPACE_DISTRHO

class JitDrumExampleUI : public WebHostUI
{
public:
    JitDrumExampleUI() : WebHostUI(BASE_WIDTH_PX, BASE_HEIGHT_PX, INIT_BACKGROUND_RGBA) {}
    ~JitDrumExampleUI() {}

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JitDrumExampleUI)

};

END_NAMESPACE_DISTRHO

#endif  // JITDRUM_EXAMPLE_UI_HPP
