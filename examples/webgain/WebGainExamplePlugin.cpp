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

#include "WebGainExamplePlugin.hpp"

USE_NAMESPACE_DISTRHO

Plugin* DISTRHO::createPlugin()
{
    return new WebGainExamplePlugin;
}

#define PARAMETER_COUNT 1
#define PROGRAM_COUNT   0
#define STATE_COUNT     0

WebGainExamplePlugin::WebGainExamplePlugin()
    : Plugin(PARAMETER_COUNT, PROGRAM_COUNT, STATE_COUNT)
    , fGain(1.f)
{}

void WebGainExamplePlugin::initParameter(uint32_t index, Parameter& parameter)
{
    parameter.hints = kParameterIsAutomable;

    switch (index)
    {
    case 0:
        parameter.name = "Gain";
        parameter.ranges.min = 0;
        parameter.ranges.max = 1.f;
        parameter.ranges.def = 1.f;
        break;
    default:
        break;
    }
}

float WebGainExamplePlugin::getParameterValue(uint32_t index) const
{
    switch (index)
    {
    case 0:
        return fGain;
    default:
        break;
    }
    return 0;
}

void WebGainExamplePlugin::setParameterValue(uint32_t index, float value)
{
    switch (index)
    {
    case 0:
        fGain = value;
        break;
    default:
        break;
    }
}

#if DISTRHO_PLUGIN_WANT_STATE

void WebGainExamplePlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    // unused
    (void)index;
    (void)stateKey;
    (void)defaultStateValue;
}

void WebGainExamplePlugin::setState(const char* key, const char* value)
{
    // unused
    (void)key;
    (void)value;
}

#if DISTRHO_PLUGIN_WANT_FULL_STATE

String WebGainExamplePlugin::getState(const char* key) const
{
    // unused
    (void)key;
    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE

#endif // DISTRHO_PLUGIN_WANT_STATE

void WebGainExamplePlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    const float* inpL = inputs[0];
    const float* inpR = inputs[1];
    float* outL = outputs[0];
    float* outR = outputs[1];

    for (uint32_t frame = 0; frame < frames; frame++) {
        outL[frame] = fGain * inpL[frame];
        outR[frame] = fGain * inpR[frame];
    }
}
