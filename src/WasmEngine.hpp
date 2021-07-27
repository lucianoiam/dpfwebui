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
#define own
#include "wasmer.h"

#include "extra/String.hpp"

START_NAMESPACE_DISTRHO

struct HostFunctionDescriptor;
struct HostFunctionContext;
class  WasmEngine;

typedef wasm_val_t WasmValue;
typedef std::vector<enum wasm_valkind_enum> WasmValueKindVector;
typedef std::vector<WasmValue> WasmValueVector;
typedef std::function<WasmValueVector(WasmValueVector&)> WasmFunction;
typedef std::vector<std::unique_ptr<HostFunctionContext>> HostFunctionContextVector;
typedef std::unordered_map<std::string, HostFunctionDescriptor> HostImportsMap;
typedef std::unordered_map<std::string, wasm_extern_t*> ExternsMap;

struct HostFunctionDescriptor
{
    WasmValueKindVector params;
    WasmValueKindVector result;
    WasmFunction        function;
};

struct HostFunctionContext
{
    WasmFunction function;
};

class WasmEngine
{
public:
    WasmEngine();
    ~WasmEngine();

    bool isStarted() { return fStarted; }

    void start(String& modulePath, HostImportsMap& hostImports);
    void stop();

    void* getMemory(WasmValue& wasmBasePtr);

    WasmValue getGlobal(String& name);
    void      setGlobal(String& name, WasmValue& value);

    WasmValueVector callModuleFunction(String& name, WasmValueVector& params);

    const char* convertWasmString(WasmValue& wasmPtr);

    static void throwWasmerLastError();

private:
    static void wasmValueTypeVector(const WasmValueKindVector& kinds, wasm_valtype_vec_t* types);

    static own wasm_trap_t* invokeHostFunction(void *env, const wasm_val_vec_t* params, wasm_val_vec_t* results);
#ifndef HIPHAP_ENABLE_WASI
    static own wasm_trap_t* assemblyScriptAbort(void *env, const wasm_val_vec_t* params, wasm_val_vec_t* results);
#endif

    bool               fStarted;
    wasm_engine_t*     fEngine;
    wasm_store_t*      fStore;
    wasm_instance_t*   fInstance;
    wasm_module_t*     fModule;
    wasm_extern_vec_t  fExports;
#ifdef HIPHAP_ENABLE_WASI
    wasi_env_t*        fWasiEnv;
#endif

    HostFunctionContextVector fHostFunctionContext;
    mutable ExternsMap        fExportsMap;

};

END_NAMESPACE_DISTRHO

#endif  // WASMENGINE_HPP
