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

#include <sstream>

#include "WasmEngine.hpp"

#include "macro.h"

USE_NAMESPACE_DISTRHO

WasmEngine::WasmEngine()
    : fStarted(false)
    , fEngine(0)
    , fStore(0)
    , fInstance(0)
    , fModule(0)
#ifdef HIPHAP_ENABLE_WASI
    , fWasiEnv(0)
#endif
{
    memset(&fExportsVec, 0, sizeof(fExportsVec));
}

WasmEngine::~WasmEngine()
{
    stop();
}

void WasmEngine::start(String& modulePath, WasmFunctionMap& hostFunctions)
{
    // -------------------------------------------------------------------------
    // Load and initialize binary module file

    FILE* file = fopen(modulePath, "rb");

    if (file == 0) {
        throw new std::runtime_error("Error opening Wasm module file");
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    wasm_byte_vec_t fileBytes;
    wasm_byte_vec_new_uninitialized(&fileBytes, fileSize);
    
    if (fread(fileBytes.data, fileSize, 1, file) != 1) {
        wasm_byte_vec_delete(&fileBytes);
        fclose(file);
        throw new std::runtime_error("Error reading Wasm module file");
    }

    fclose(file);

    fEngine = wasm_engine_new();

    if (fEngine == 0) {
        wasm_byte_vec_delete(&fileBytes);
        throwWasmerLastError();
    }

    fStore = wasm_store_new(fEngine);

    if (fEngine == 0) {
        wasm_byte_vec_delete(&fileBytes);
        throwWasmerLastError();
    }

    fModule = wasm_module_new(fStore, &fileBytes);
    
    wasm_byte_vec_delete(&fileBytes);

    if (fModule == 0) {
        throwWasmerLastError();
    }

    char name[128];

#ifdef HIPHAP_ENABLE_WASI
    // -------------------------------------------------------------------------
    // Build a map of WASI imports
    // Call to wasi_get_imports() fails because of missing host imports, use
    // wasi_get_unordered_imports() https://github.com/wasmerio/wasmer/issues/2450

    wasi_config_t* config = wasi_config_new("DPF");
    fWasiEnv = wasi_env_new(config);

    if (fWasiEnv == 0) {
        throwWasmerLastError();
    }

    wasmer_named_extern_vec_t wasiImports;

    if (!wasi_get_unordered_imports(fStore, fModule, fWasiEnv, &wasiImports)) {
        throwWasmerLastError();
    }

    std::unordered_map<std::string, int> wasiImportIndex;

    for (size_t i = 0; i < wasiImports.size; i++) {
        wasmer_named_extern_t *ne = wasiImports.data[i];
        const wasm_name_t *wn = wasmer_named_extern_name(ne);
        memcpy(name, wn->data, wn->size);
        name[wn->size] = 0;
        wasiImportIndex[name] = i;
    }
#endif // HIPHAP_ENABLE_WASI

    // -------------------------------------------------------------------------
    // Build module imports vector

    wasm_importtype_vec_t importTypes;
    wasm_module_imports(fModule, &importTypes);
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_uninitialized(&imports, importTypes.size);

    std::unordered_map<std::string, int> importIndex;
    bool moduleNeedsWasi = false;

    for (size_t i = 0; i < importTypes.size; i++) {
        const wasm_name_t *wn = wasm_importtype_name(importTypes.data[i]);
        memcpy(name, wn->data, wn->size);
        name[wn->size] = 0;
        importIndex[name] = i;

#ifdef HIPHAP_ENABLE_WASI
        if (wasiImportIndex.find(name) != wasiImportIndex.end()) {
            wasmer_named_extern_t* ne = wasiImports.data[wasiImportIndex[name]];
            imports.data[i] = const_cast<wasm_extern_t *>(wasmer_named_extern_unwrap(ne));
        }
#endif
        if (!moduleNeedsWasi) {
            wn = wasm_importtype_module(importTypes.data[i]);
            memcpy(name, wn->data, wn->size);
            name[wn->size] = 0;
            if (strstr(name, "wasi_") == name) { // eg, wasi_snapshot_preview1
                moduleNeedsWasi = true;
            }
        }
    }

    wasm_importtype_vec_delete(&importTypes);

#ifdef HIPHAP_ENABLE_WASI
    if (!moduleNeedsWasi) {
        throw new std::runtime_error("WASI is enabled but module is not WASI compliant");
    }
#else
    if (moduleNeedsWasi) {
        throw new std::runtime_error("WASI is not enabled but module requires WASI");
    }
#endif

    // -------------------------------------------------------------------------
    // Insert host functions into imports vector

#ifndef HIPHAP_ENABLE_WASI
    // Required by AssemblyScript when running in non-WASI mode
    hostFunctions["abort"] = { {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}, 
        std::bind(&WasmEngine::assemblyScriptAbort, this, std::placeholders::_1) };
#endif

    for (WasmFunctionMap::const_iterator it = hostFunctions.begin(); it != hostFunctions.end(); ++it) {
        std::string name = it->first;
        const WasmValueKindVector& paramsKind = it->second.params;
        const WasmValueKindVector& resultKind = it->second.result;

        wasm_valtype_vec_t paramsType;
        toWasmValueTypeVector(paramsKind, &paramsType);
        wasm_valtype_vec_t resultType;
        toWasmValueTypeVector(resultKind, &resultType);

        wasm_functype_t* funcType = wasm_functype_new(&paramsType, &resultType);
        fHostFunctions.push_back(it->second.function);
        wasm_func_t* func = wasm_func_new_with_env(fStore, funcType, WasmEngine::invokeHostFunction,
                                                    &fHostFunctions.back(), 0);
        imports.data[importIndex[name]] = wasm_func_as_extern(func);
    }

    // -------------------------------------------------------------------------
    // Create Wasm instance and start WASI

    fInstance = wasm_instance_new(fStore, fModule, &imports, 0);

    wasm_extern_vec_delete(&imports);

    if (fInstance == 0) {
        throwWasmerLastError();
    }
#ifdef HIPHAP_ENABLE_WASI
    wasm_func_t* wasiStart = wasi_get_start_function(fInstance);
    
    if (wasiStart == 0) {
        throwWasmerLastError();
    }

    wasm_val_vec_t empty_val_vec = WASM_EMPTY_VEC;
    wasm_func_call(wasiStart, &empty_val_vec, &empty_val_vec);
    wasm_func_delete(wasiStart);
#endif
    // -------------------------------------------------------------------------
    // Build a map of externs indexed by name

    fExportsVec.size = 0;
    wasm_instance_exports(fInstance, &fExportsVec);
    wasm_exporttype_vec_t exportTypes;
    wasm_module_exports(fModule, &exportTypes);

    for (size_t i = 0; i < fExportsVec.size; i++) {
        const wasm_name_t *wn = wasm_exporttype_name(exportTypes.data[i]);
        memcpy(name, wn->data, wn->size);
        name[wn->size] = 0;
        fModuleExports[name] = fExportsVec.data[i];
    }

    wasm_exporttype_vec_delete(&exportTypes);

    // -------------------------------------------------------------------------
    // Startup complete

    fStarted = true;
}

void WasmEngine::stop()
{
#ifdef HIPHAP_ENABLE_WASI
    if (fWasiEnv != 0) {
        wasi_env_delete(fWasiEnv);
        fWasiEnv = 0;
    }
#endif
    if (fExportsVec.size != 0) {
        wasm_extern_vec_delete(&fExportsVec);
        fExportsVec.size = 0;
    }

    if (fModule != 0) {
        wasm_module_delete(fModule);
        fModule = 0;
    }

    if (fInstance != 0) {
        wasm_instance_delete(fInstance);
        fInstance = 0;
    }

    if (fStore != 0) {
        wasm_store_delete(fStore);
        fStore = 0;
    }

    if (fEngine != 0) {
        wasm_engine_delete(fEngine);
        fEngine = 0;
    }

    fHostFunctions.clear();
    fModuleExports.clear();

    fStarted = false;
}

void* WasmEngine::getMemory(const WasmValue& wasmBasePtr)
{
    return wasm_memory_data(wasm_extern_as_memory(fModuleExports["memory"])) + wasmBasePtr.of.i32;
}

WasmValue WasmEngine::getGlobal(String& name)
{
    WasmValue value;
    wasm_global_get(wasm_extern_as_global(fModuleExports[name.buffer()]), &value);
    return value;
}

void WasmEngine::setGlobal(String& name, const WasmValue& value)
{
    wasm_global_set(wasm_extern_as_global(fModuleExports[name.buffer()]), &value);
}

WasmValueVector WasmEngine::callModuleFunction(String& name, const WasmValueVector& params)
{
    // TODO
    (void)name;
    (void)params;
    WasmValueVector res;
    return res;
}

String WasmEngine::fromWasmString(const WasmValue& wasmPtr)
{
    if (wasmPtr.of.i32 == 0) {
        return String("(null)");
    }

    wasm_val_t paramsArray[1] = { wasmPtr };
    wasm_val_vec_t params = WASM_ARRAY_VEC(paramsArray);
    wasm_val_t resultArray[1] = { WASM_INIT_VAL };
    wasm_val_vec_t result = WASM_ARRAY_VEC(resultArray);
    wasm_func_t* func = wasm_extern_as_func(fModuleExports["_c_string"]);

    if (wasm_func_call(func, &params, &result) != 0) {
        throwWasmerLastError();
    }

    return String(static_cast<const char *>(getMemory(resultArray[0])));
}

void WasmEngine::throwWasmerLastError()
{
    int len = wasmer_last_error_length();
    
    if (len == 0) {
        throw new std::runtime_error("Wasmer unknown error");
    }

    char msg[len];
    wasmer_last_error_message(msg, len);

    throw new std::runtime_error(std::string("Wasmer error - ") + msg);
}

void WasmEngine::toWasmValueTypeVector(const WasmValueKindVector& kinds, wasm_valtype_vec_t *types)
{
    int i = 0;
    int size = kinds.size();
    wasm_valtype_t* typesArray[size];

    for (WasmValueKindVector::const_iterator it = kinds.cbegin(); it != kinds.cend(); ++it) {
        typesArray[i++] = wasm_valtype_new(*it);
    }

    wasm_valtype_vec_new(types, size, typesArray);
}

own wasm_trap_t* WasmEngine::invokeHostFunction(void* env, const wasm_val_vec_t* params, wasm_val_vec_t* results)
{
    WasmFunction* f = static_cast<WasmFunction *>(env);
    
    // TODO

    // Convert wasm_val_vec_t to WasmValueVector
    // Call
    // Throw if needed
    // Convert WasmValueVector to wasm_val_vec_t

    (void)f;
    (void)params;
    (void)results;

    return 0;
}

#ifndef HIPHAP_ENABLE_WASI

WasmValueVector WasmEngine::assemblyScriptAbort(const WasmValueVector& params)
{
    const char *msg = fromWasmString(params[0]);
    const char *filename = fromWasmString(params[1]);
    int32_t lineNumber = params[2].of.i32;
    int32_t columnNumber = params[3].of.i32;

    std::stringstream ss;
    ss << "AssemblyScript abort() called - msg: " << msg << ", filename: " << filename
        << ", lineNumber: " << lineNumber << ", columnNumber: " << columnNumber;

    HIPHAP_LOG_STDERR_COLOR(ss.str().c_str());

    return {};
}

#endif // HIPHAP_ENABLE_WASI
