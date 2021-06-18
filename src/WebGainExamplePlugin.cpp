/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "WebGainExamplePlugin.hpp"

USE_NAMESPACE_DISTRHO

Plugin* DISTRHO::createPlugin()
{
    return new WebGainExamplePlugin;
}

WebGainExamplePlugin::WebGainExamplePlugin()
    : Plugin(1 /* parameterCount */, 0 /* programCount */, 0 /* stateCount */)
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

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

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

#if (DISTRHO_PLUGIN_WANT_FULL_STATE == 1)

String WebGainExamplePlugin::getState(const char* key) const
{
    // unused
    (void)key;
    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE == 1

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

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
