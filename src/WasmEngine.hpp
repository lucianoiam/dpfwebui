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

#ifndef WASMENGINE_HPP
#define WASMENGINE_HPP

#include <string>
#include <unordered_map>
#include <vector>

#define WASM_API_EXTERN // link to static lib on win32
#include "wasm.h"
#include "wasmer.h"

#include "extra/String.hpp"

START_NAMESPACE_DISTRHO

struct HostFunctionDescriptor;

typedef std::vector<wasm_val_t> WasmValueVector;
typedef std::vector<enum wasm_valkind_enum> WasmValueKindVector;
typedef std::function<WasmValueVector(WasmValueVector&)> WasmFunction;
typedef std::unordered_map<std::string, HostFunctionDescriptor> HostImportMap;
typedef std::unordered_map<std::string, wasm_extern_t*> ExternMap;

struct HostFunctionDescriptor
{
    WasmValueKindVector arguments;
    WasmValueKindVector result;
    WasmFunction        function;
};

class WasmEngine
{
public:
    WasmEngine();
    ~WasmEngine();

    bool isStarted() { return fStarted; }

    void start(String& modulePath, HostImportMap& hostImports);
    void stop();

    wasm_val_t getGlobal(String& name);
    void       setGlobal(String& name, wasm_val_t& value);

    WasmValueVector callFunction(String& name, WasmValueVector& args);

    void throwWasmerLastError();

    const char* readWasmString(int32_t wasmPtr);

private:
    bool               fStarted;
    wasm_engine_t*     fEngine;
    wasm_store_t*      fStore;
    wasm_instance_t*   fInstance;
    wasm_module_t*     fModule;
    wasm_extern_vec_t  fExports;
#ifdef HIPHAP_ENABLE_WASI
    wasi_env_t*        fWasiEnv;
#endif

    mutable ExternMap  fExportsMap;

};

END_NAMESPACE_DISTRHO

#endif  // WASMENGINE_HPP
