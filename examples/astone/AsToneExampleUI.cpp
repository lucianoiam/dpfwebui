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

#include "AsToneExampleUI.hpp"

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new AsToneExampleUI;
}

AsToneExampleUI::AsToneExampleUI()
    : UI(400, 300)
    , fBlendish(this)
    , fKnob(&fBlendish)
{
    glClearColor(0.9f, 0.9f, 0.9f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    fBlendish.setSize(getSize());

    fKnob.setAbsoluteX((getWidth() - fKnob.getWidth()) / 2);
    fKnob.setAbsoluteY((getHeight() - fKnob.getHeight()) / 3);
    fKnob.setRange(220.f, 880.f);
    fKnob.setValue(440.f);
    fKnob.setUnit("Hz");
    fKnob.setCallback(this);
}

void AsToneExampleUI::knobValueChanged(SubWidget*, float value)
{
    setParameterValue(0, value);
}

void AsToneExampleUI::parameterChanged(uint32_t index, float value)
{
    switch (index) {
        case 0:
            fKnob.setValue(value);
            break;
    }
}
