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

#define MODULE_NAME_MAX 128

USE_NAMESPACE_DISTRHO

WasmEngine::WasmEngine()
    : fStarted(false)
    , fEngine(0)
    , fStore(0)
    , fInstance(0)
    , fModule(0)
#ifdef HIPHOP_ENABLE_WASI
    , fWasiEnv(0)
#endif
{
    memset(&fExportsVec, 0, sizeof(fExportsVec));
}

WasmEngine::~WasmEngine()
{
    stop();
}

void WasmEngine::start(String& modulePath, WasmFunctionMap hostFunctions)
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

    char name[MODULE_NAME_MAX];

#ifdef HIPHOP_ENABLE_WASI
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
#endif // HIPHOP_ENABLE_WASI

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

#ifdef HIPHOP_ENABLE_WASI
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

#ifdef HIPHOP_ENABLE_WASI
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

#ifndef HIPHOP_ENABLE_WASI
    // Required by AssemblyScript when running in non-WASI mode
    hostFunctions["abort"] = { {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}, 
        std::bind(&WasmEngine::assemblyScriptAbort, this, std::placeholders::_1) };
#endif

    for (WasmFunctionMap::const_iterator it = hostFunctions.begin(); it != hostFunctions.end(); ++it) {
        fHostFunctions.push_back(it->second.function);

        wasm_valtype_vec_t params;
        toCValueTypeVector(it->second.params, &params);
        wasm_valtype_vec_delete(&params);
        
        wasm_valtype_vec_t result;
        toCValueTypeVector(it->second.result, &result);
        wasm_valtype_vec_delete(&result);
        
        wasm_functype_t* funcType = wasm_functype_new(&params, &result);
        wasm_func_t* func = wasm_func_new_with_env(fStore, funcType, WasmEngine::invokeHostFunction,
                                                    &fHostFunctions.back(), 0);
        imports.data[importIndex[it->first]] = wasm_func_as_extern(func);
    }

    // -------------------------------------------------------------------------
    // Create Wasm instance and start WASI

    fInstance = wasm_instance_new(fStore, fModule, &imports, 0);

    wasm_extern_vec_delete(&imports);

    if (fInstance == 0) {
        throwWasmerLastError();
    }
#ifdef HIPHOP_ENABLE_WASI
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
#ifdef HIPHOP_ENABLE_WASI
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
    byte_t* ptr = wasm_memory_data(wasm_extern_as_memory(fModuleExports["memory"]));
    return ptr + wasmBasePtr.of.i32;
}

WasmValue WasmEngine::getGlobal(const char* name)
{
    WasmValue value;
    wasm_global_get(wasm_extern_as_global(fModuleExports[name]), &value);
    return value;
}

void WasmEngine::setGlobal(const char* name, const WasmValue& value)
{
    wasm_global_set(wasm_extern_as_global(fModuleExports[name]), &value);
}

String WasmEngine::fromWasmString(const WasmValue& wasmPtr)
{
    if (wasmPtr.of.i32 == 0) {
        return String("(null)");
    }

    if (fModuleExports.find("_c_string") == fModuleExports.end()) {
        throw new std::runtime_error("Wasm module does not export function _c_string");
    }

    WasmValueVector result = callModuleFunction("_c_string", { wasmPtr });

    return String(static_cast<const char *>(getMemory(result[0])));
}

WasmValueVector WasmEngine::callModuleFunction(const char* name, WasmValueVector params)
{
    wasm_func_t* func = wasm_extern_as_func(fModuleExports[name]);

    wasm_val_vec_t paramsVec;
    paramsVec.size = params.size();
    paramsVec.data = params.data();

    wasm_val_t resultArray[1] = { WASM_INIT_VAL };
    wasm_val_vec_t resultVec = WASM_ARRAY_VEC(resultArray);

    wasm_trap_t* trap = wasm_func_call(func, &paramsVec, &resultVec);

    if (trap != 0) {
        throwWasmerLastError();
    }

    return WasmValueVector(resultVec.data, resultVec.data + resultVec.size);
}

wasm_trap_t* WasmEngine::invokeHostFunction(void* env, const wasm_val_vec_t* paramsVec, wasm_val_vec_t* resultVec)
{
    WasmFunction* func = static_cast<WasmFunction *>(env);
    WasmValueVector params (paramsVec->data, paramsVec->data + paramsVec->size);
    WasmValueVector result = (*func)(params);

    for (size_t i = 0; i < resultVec->size; i++) {
        resultVec->data[i] = result[i];
    }

    return 0;
}

void WasmEngine::toCValueTypeVector(WasmValueKindVector kinds, wasm_valtype_vec_t* types)
{
    int i = 0;
    size_t size = kinds.size();
    wasm_valtype_t* typesArray[size];

    for (WasmValueKindVector::const_iterator it = kinds.cbegin(); it != kinds.cend(); ++it) {
        typesArray[i++] = wasm_valtype_new(*it);
    }

    wasm_valtype_vec_new(types, size, typesArray);
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

#ifndef HIPHOP_ENABLE_WASI

WasmValueVector WasmEngine::assemblyScriptAbort(WasmValueVector params)
{
    const char *msg = fromWasmString(params[0]);
    const char *filename = fromWasmString(params[1]);
    int32_t lineNumber = params[2].of.i32;
    int32_t columnNumber = params[3].of.i32;

    std::stringstream ss;
    ss << "AssemblyScript abort() called - msg: " << msg << ", filename: " << filename
        << ", lineNumber: " << lineNumber << ", columnNumber: " << columnNumber;

    HIPHOP_LOG_STDERR_COLOR(ss.str().c_str());

    return {};
}

#endif // HIPHOP_ENABLE_WASI
