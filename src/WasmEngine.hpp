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

struct WasmFunctionDescriptor;

typedef wasm_val_t WasmValue;
typedef std::vector<WasmValue> WasmValueVector;
typedef std::vector<enum wasm_valkind_enum> WasmValueKindVector;
typedef std::function<WasmValueVector(const WasmValueVector&)> WasmFunction;
typedef std::vector<WasmFunction> WasmFunctionVector;
typedef std::unordered_map<std::string, WasmFunctionDescriptor> WasmFunctionMap;
typedef std::unordered_map<std::string, wasm_extern_t*> WasmExternMap;

struct WasmFunctionDescriptor
{
    WasmValueKindVector params;
    WasmValueKindVector result;
    WasmFunction        function;
};

class WasmEngine
{
public:
    WasmEngine();
    ~WasmEngine();

    bool isStarted() { return fStarted; }

    void start(String& modulePath, WasmFunctionMap& hostFunctions);
    void stop();

    void* getMemory(const WasmValue& wasmBasePtr);

    WasmValue getGlobal(String& name);
    void      setGlobal(String& name, const WasmValue& value);

    WasmValueVector callModuleFunction(String& name, const WasmValueVector& params);

    String fromWasmString(const WasmValue& wasmPtr);

private:
    static void throwWasmerLastError();
    static void toWasmValueTypeVector(const WasmValueKindVector& kinds, wasm_valtype_vec_t* types);

    static wasm_trap_t* invokeHostFunction(void *env, const wasm_val_vec_t* params, wasm_val_vec_t* results);

#ifndef HIPHOP_ENABLE_WASI
    WasmValueVector assemblyScriptAbort(const WasmValueVector& params);
#endif

    bool               fStarted;
    wasm_engine_t*     fEngine;
    wasm_store_t*      fStore;
    wasm_instance_t*   fInstance;
    wasm_module_t*     fModule;
    wasm_extern_vec_t  fExportsVec;
#ifdef HIPHOP_ENABLE_WASI
    wasi_env_t*        fWasiEnv;
#endif
    WasmFunctionVector fHostFunctions;
    WasmExternMap      fModuleExports;

};

END_NAMESPACE_DISTRHO

#endif  // WASMENGINE_HPP
