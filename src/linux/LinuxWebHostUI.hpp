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

#ifndef LINUX_WEB_HOST_UI_HPP
#define LINUX_WEB_HOST_UI_HPP

#include "AbstractWebHostUI.hpp"
#include "ExternalGtkWebView.hpp"

START_NAMESPACE_DISTRHO

class LinuxWebHostUI : public AbstractWebHostUI
{
public:
    LinuxWebHostUI(uint baseWidth = 0, uint baseHeight = 0, uint32_t backgroundColor = 0xffffffff);
    virtual ~LinuxWebHostUI();

protected:
    uintptr_t createStandaloneWindow() override;
    void      processStandaloneEvents() override;

    AbstractWebView& getWebView() override { return fWebView; }

private:
    ExternalGtkWebView fWebView;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinuxWebHostUI)

};

END_NAMESPACE_DISTRHO

#endif  // LINUX_WEB_HOST_UI_HPP
