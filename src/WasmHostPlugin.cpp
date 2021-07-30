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

#include "WasmHostPlugin.hpp"

#include "Platform.hpp"
#include "macro.h"

#define ERROR_STR "Error"

USE_NAMESPACE_DISTRHO

WasmHostPlugin::WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
    : Plugin(parameterCount, programCount, stateCount)
{   
    String path = platform::getLibraryPath() + "/dsp/plugin.wasm";
    WasmFunctionMap hostFunctions;

    hostFunctions["_get_sample_rate"] = { {}, { WASM_F32 }, [this](WasmValueVector) -> WasmValueVector {
        return { MakeF32(getSampleRate()) };
    }};

    fEngine.start(path, hostFunctions);

    fEngine.setGlobal("_num_inputs", MakeI32(DISTRHO_PLUGIN_NUM_INPUTS));
    fEngine.setGlobal("_num_outputs", MakeI32(DISTRHO_PLUGIN_NUM_OUTPUTS));
}

const char* WasmHostPlugin::getLabel() const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnCString("_get_label");
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return ERROR_STR;
}

const char* WasmHostPlugin::getMaker() const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnCString("_get_maker");
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return ERROR_STR;
}

const char* WasmHostPlugin::getLicense() const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnCString("_get_license");
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return ERROR_STR;
}

uint32_t WasmHostPlugin::getVersion() const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnSingleValue("_get_version").of.i32;
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return 0;
}

int64_t WasmHostPlugin::getUniqueId() const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnSingleValue("_get_unique_id").of.i64;
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return 0;
}

void WasmHostPlugin::initParameter(uint32_t index, Parameter& parameter)
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_init_parameter", { MakeI32(index) });
        WasmValue res;
        res = fEngine.getGlobal("_rw_int_1");    parameter.hints = res.of.i32;
        res = fEngine.getGlobal("_ro_string_1"); parameter.name = fEngine.getMemoryAsCString(res);
        res = fEngine.getGlobal("_rw_float_1");  parameter.ranges.def = res.of.f32;
        res = fEngine.getGlobal("_rw_float_2");  parameter.ranges.min = res.of.f32;
        res = fEngine.getGlobal("_rw_float_3");  parameter.ranges.max = res.of.f32;
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

float WasmHostPlugin::getParameterValue(uint32_t index) const
{
    try {
        checkEngineStarted();
        return fEngine.callFunctionReturnSingleValue("_get_parameter_value",
            { MakeI32(index) }).of.f32;
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return 0;
}

void WasmHostPlugin::setParameterValue(uint32_t index, float value)
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_set_parameter_value", { MakeI32(index), MakeF32(value) });
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

#if DISTRHO_PLUGIN_WANT_PROGRAMS

void WasmHostPlugin::initProgramName(uint32_t index, String& programName)
{
    try {
        checkEngineStarted();
        programName = fEngine.callFunctionReturnCString("_init_program_name", { MakeI32(index) });
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

void WasmHostPlugin::loadProgram(uint32_t index)
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_load_program", { MakeI32(index) });
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

#endif // DISTRHO_PLUGIN_WANT_PROGRAMS

#if DISTRHO_PLUGIN_WANT_STATE

void WasmHostPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_init_state", { MakeI32(index) });
        stateKey = fEngine.getMemoryAsCString(fEngine.getGlobal("_ro_string_1"));
        defaultStateValue = fEngine.getMemoryAsCString(fEngine.getGlobal("_ro_string_2"));
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

void WasmHostPlugin::setState(const char* key, const char* value)
{
    try {
        checkEngineStarted();
        WasmValue wkey = fEngine.getGlobal("_rw_string_1");
        fEngine.copyCStringToMemory(wkey, key);
        WasmValue wval = fEngine.getGlobal("_rw_string_2");
        fEngine.copyCStringToMemory(wval, value);
        fEngine.callFunction("_set_state", { wkey, wval });
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

#if DISTRHO_PLUGIN_WANT_FULL_STATE

String WasmHostPlugin::getState(const char* key) const
{
    try {
        checkEngineStarted();
        WasmValue wkey = fEngine.getGlobal("_rw_string_1");
        fEngine.copyCStringToMemory(wkey, key);
        const char* val = fEngine.callFunctionReturnCString("_get_state", { wkey });
        return String(val);
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }

    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE

#endif // DISTRHO_PLUGIN_WANT_STATE

void WasmHostPlugin::activate()
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_activate");
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

void WasmHostPlugin::deactivate()
{
    try {
        checkEngineStarted();
        fEngine.callFunction("_deactivate");
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

void WasmHostPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    try {
        checkEngineStarted();

        float32_t* block;

        block = reinterpret_cast<float32_t *>(fEngine.getMemory(fEngine.getGlobal("_input_block")));

        for (int i = 0; i < DISTRHO_PLUGIN_NUM_INPUTS; i++) {
            memcpy(block + i * frames, inputs[i], frames * 4);
        }

        fEngine.callFunction("_run", { MakeI32(frames) });

        block = reinterpret_cast<float32_t *>(fEngine.getMemory(fEngine.getGlobal("_output_block")));

        for (int i = 0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; i++) {
            memcpy(outputs[i], block + i * frames, frames * 4);
        }
    } catch (const std::exception& ex) {
        HIPHOP_LOG_STDERR_COLOR(ex.what());
    }
}

void WasmHostPlugin::checkEngineStarted() const
{
    if (!fEngine.isStarted()) {
        throw new std::runtime_error("Wasm engine is not running");
    }
}
