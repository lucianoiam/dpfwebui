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

#ifndef WEBGAIN_EXAMPLE_PLUGIN_HPP
#define WEBGAIN_EXAMPLE_PLUGIN_HPP

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class WebGainExamplePlugin : public Plugin
{
public:
    WebGainExamplePlugin();
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

    void  initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

#if DISTRHO_PLUGIN_WANT_STATE
    void   initState(uint32_t index, String& stateKey, String& defaultStateValue) override;
    void   setState(const char* key, const char* value) override;
#if DISTRHO_PLUGIN_WANT_FULL_STATE
    String getState(const char* key) const override;
#endif
#endif

    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:
    float fGain;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebGainExamplePlugin)

};

END_NAMESPACE_DISTRHO

#endif  // WEBGAIN_EXAMPLE_PLUGIN_HPP
