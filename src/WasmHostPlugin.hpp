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

#ifndef WASM_HOST_PLUGIN_HPP
#define WASM_HOST_PLUGIN_HPP

#include "DistrhoPlugin.hpp"

#include "WasmEngine.hpp"

START_NAMESPACE_DISTRHO

class WasmHostPlugin : public Plugin
{
public:
    WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount);
    ~WasmHostPlugin() {};

    const char* getLabel() const override;
    const char* getMaker() const override;
    const char* getLicense() const override;

    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;

    void  initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

#if DISTRHO_PLUGIN_WANT_PROGRAMS
    void initProgramName(uint32_t index, String& programName) override;
    void loadProgram(uint32_t index) override;
#endif

#if DISTRHO_PLUGIN_WANT_STATE
    void   initState(uint32_t index, String& stateKey, String& defaultStateValue) override;
    void   setState(const char* key, const char* value) override;
#if DISTRHO_PLUGIN_WANT_FULL_STATE
    String getState(const char* key) const override;
#endif
#endif

    void activate() override;
    void deactivate() override;

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    void run(const float** inputs, float** outputs, uint32_t frames,
                const MidiEvent* midiEvents, uint32_t midiEventCount) override;
#else
    void run(const float** inputs, float** outputs, uint32_t frames) override;
#endif

private:
    inline void throwIfEngineStopped() const;

    WasmValueVector writeMidiEvent(WasmValueVector params);

    mutable WasmEngine fEngine;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WasmHostPlugin)

};

END_NAMESPACE_DISTRHO

#endif  // WASM_HOST_PLUGIN_HPP
