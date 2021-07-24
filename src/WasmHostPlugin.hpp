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

#ifndef WASMHOSTPLUGIN_HPP
#define WASMHOSTPLUGIN_HPP

#import <unordered_map>
#import <string>

#define WASM_API_EXTERN // link to static lib on win32
#include "wasm.h"
#include "wasmer.h"

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class WasmHostPlugin : public Plugin
{
public:
    WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount);
    ~WasmHostPlugin();

    const char* getLabel() const override;
    const char* getMaker() const override;
    const char* getLicense() const override;

    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;

    void  initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

#if (DISTRHO_PLUGIN_WANT_STATE == 1)
    void   initState(uint32_t index, String& stateKey, String& defaultStateValue) override;
    void   setState(const char* key, const char* value) override;
#if (DISTRHO_PLUGIN_WANT_FULL_STATE == 1)
    String getState(const char* key) const override;
#endif
#endif

    void activate() override;
    void deactivate() override;

    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:
    bool               fWasmReady;
    wasm_engine_t*     fWasmEngine;
    wasm_store_t*      fWasmStore;
    wasm_instance_t*   fWasmInstance;
    wasm_module_t*     fWasmModule;
    wasi_env_t*        fWasiEnv;
    wasm_extern_vec_t  fWasmExports;
    byte_t*            fWasmMemoryBytes;
    float32_t*         fInputBlock;
    float32_t*         fOutputBlock;

    typedef std::unordered_map<std::string, wasm_extern_t *> ExternMap;
    mutable ExternMap fExternMap;

};

END_NAMESPACE_DISTRHO

#endif  // WASMHOSTPLUGIN_HPP
