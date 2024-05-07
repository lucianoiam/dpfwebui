/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
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

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class WebGainExamplePlugin : public Plugin
{
public:
    WebGainExamplePlugin()
        : Plugin(1 /*parameters*/, 0 /*programs*/, 0 /*states*/)
        , fGain(1.f)
    {}

    ~WebGainExamplePlugin() {}

    const char* getLabel() const override
    {
        return "WebGain";
    }

    const char* getMaker() const override
    {
        return "Luciano Iam";
    }

    const char* getLicense() const override
    {
        return "GPLv3";
    }

    uint32_t getVersion() const override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const override
    {
        return d_cconst('H', 'H', 'w', 'g');
    }

    // VST3
    void initAudioPort(const bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        parameter.hints = kParameterIsAutomatable;

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

    float getParameterValue(uint32_t index) const override
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

    void setParameterValue(uint32_t index, float value) override
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
    void initState(uint32_t index, State& state) override
    {
        // unused
        (void)index;
        (void)stateKey;
        (void)defaultStateValue;
    }

    void setState(const char* key, const char* value) override
    {
        // unused
        (void)key;
        (void)value;
    }

#if DISTRHO_PLUGIN_WANT_FULL_STATE
    String getState(const char* key) const override
    {
        // unused
        (void)key;
        return String();
    }
#endif // DISTRHO_PLUGIN_WANT_FULL_STATE
#endif // DISTRHO_PLUGIN_WANT_STATE

    void run(const float** inputs, float** outputs, uint32_t frames) override
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

private:
    float fGain;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebGainExamplePlugin)

};

Plugin* createPlugin()
{
    return new WebGainExamplePlugin;
}

END_NAMESPACE_DISTRHO
