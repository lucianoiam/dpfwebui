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

#include <iostream>

#include "WasmEngine.hpp"

#define MAX_MODULE_NAME    128
#define MAX_HOST_FUNCTIONS 128

USE_NAMESPACE_DISTRHO

WasmEngine::WasmEngine()
    : fStarted(false)
    , fEngine(0)
    , fStore(0)
    , fModule(0)
    , fInstance(0)
#ifdef HIPHOP_ENABLE_WASI
    , fWasiEnv(0)
#endif
{
    memset(&fExportsVec, 0, sizeof(fExportsVec));
}

WasmEngine::~WasmEngine()
{
    stop();
    unload();
}

void WasmEngine::load(const char* modulePath)
{
    FILE* file = fopen(modulePath, "rb");

    if (file == 0) {
        throw std::runtime_error("Error opening Wasm module file");
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    wasm_byte_vec_t fileBytes;
    wasm_byte_vec_new_uninitialized(&fileBytes, fileSize);
    
    if (fread(fileBytes.data, fileSize, 1, file) != 1) {
        wasm_byte_vec_delete(&fileBytes);
        fclose(file);
        throw std::runtime_error("Error reading Wasm module file");
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
}

void WasmEngine::unload()
{
    if (fModule != 0) {
        wasm_module_delete(fModule);
        fModule = 0;
    }
    
    if (fStore != 0) {
        wasm_store_delete(fStore);
        fStore = 0;
    }

    if (fEngine != 0) {
        wasm_engine_delete(fEngine);
        fEngine = 0;
    }
}

void WasmEngine::start(WasmFunctionMap hostFunctions)
{
    char name[MAX_MODULE_NAME];

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
        throw std::runtime_error("WASI is enabled but module is not WASI compliant");
    }
#else
    if (moduleNeedsWasi) {
        throw std::runtime_error("WASI is not enabled but module requires WASI");
    }
#endif

    // -------------------------------------------------------------------------
    // Insert host functions into imports vector

    // Avoid reallocation to ensure pointers to elements remain valid through engine lifetime
    fHostFunctions.reserve(MAX_HOST_FUNCTIONS);

#ifndef HIPHOP_ENABLE_WASI
    hostFunctions["abort"] = { { WASM_I32, WASM_I32, WASM_I32, WASM_I32 }, {}, 
        std::bind(&WasmEngine::nonWasiAssemblyScriptAbort, this, std::placeholders::_1) };
#endif

    for (WasmFunctionMap::const_iterator it = hostFunctions.begin(); it != hostFunctions.end(); ++it) {
        fHostFunctions.push_back(it->second.function);

        wasm_valtype_vec_t params;
        toCValueTypeVector(it->second.params, &params);
        wasm_valtype_vec_t result;
        toCValueTypeVector(it->second.result, &result);

        wasm_functype_t* funcType = wasm_functype_new(&params, &result);
        wasm_func_t* func = wasm_func_new_with_env(fStore, funcType, WasmEngine::callHostFunction,
                                                    &fHostFunctions.back(), 0);
        imports.data[importIndex[it->first]] = wasm_func_as_extern(func);

        wasm_valtype_vec_delete(&result);
        wasm_valtype_vec_delete(&params);
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

    if (fInstance != 0) {
        wasm_instance_delete(fInstance);
        fInstance = 0;
    }

    fHostFunctions.clear();
    fModuleExports.clear();

    fStarted = false;
}

byte_t* WasmEngine::getMemory(const WasmValue& wPtr)
{
    return wasm_memory_data(wasm_extern_as_memory(fModuleExports["memory"])) + wPtr.of.i32;
}

char* WasmEngine::getMemoryAsCString(const WasmValue& wPtr)
{
    return static_cast<char *>(getMemory(wPtr));
}

void WasmEngine::copyCStringToMemory(const WasmValue& wPtr, const char* s)
{
    strcpy(getMemoryAsCString(wPtr), s);
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

char* WasmEngine::getGlobalAsCString(const char* name)
{
    return getMemoryAsCString(getGlobal(name));
}

WasmValueVector WasmEngine::callFunction(const char* name, WasmValueVector params)
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

WasmValue WasmEngine::callFunctionReturnSingleValue(const char* name, WasmValueVector params)
{
    return callFunction(name, params)[0];
}

const char* WasmEngine::callFunctionReturnCString(const char* name, WasmValueVector params)
{
    return getMemoryAsCString(callFunctionReturnSingleValue(name, params));
}

wasm_trap_t* WasmEngine::callHostFunction(void* env, const wasm_val_vec_t* paramsVec, wasm_val_vec_t* resultVec)
{
    WasmFunction* func = static_cast<WasmFunction *>(env);
    WasmValueVector params (paramsVec->data, paramsVec->data + paramsVec->size);
    WasmValueVector result = (*func)(params);

    for (size_t i = 0; i < resultVec->size; i++) {
        resultVec->data[i] = result[i];
    }

    return 0;
}

void WasmEngine::throwWasmerLastError()
{
    int len = wasmer_last_error_length();
    
    if (len == 0) {
        throw std::runtime_error("Wasmer unknown error");
    }

    char msg[len];
    wasmer_last_error_message(msg, len);

    throw std::runtime_error(std::string("Wasmer error - ") + msg);
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

const char* WasmEngine::WTF16ToCString(const WasmValue& wPtr)
{
    if (fModuleExports.find("_wtf16_to_c_string") == fModuleExports.end()) {
        throw std::runtime_error("Wasm module does not export function _wtf16_to_c_string");
    }

    return callFunctionReturnCString("_wtf16_to_c_string", { wPtr });

}

WasmValue WasmEngine::CToWTF16String(const char* s)
{
    if (fModuleExports.find("_c_to_wtf16_string") == fModuleExports.end()) {
        throw std::runtime_error("Wasm module does not export function _c_to_wtf16_string");
    }

    WasmValue wPtr = getGlobal("_rw_string_1");

    copyCStringToMemory(wPtr, s);

    return callFunctionReturnSingleValue("_c_to_wtf16_string", { wPtr });
}

#ifndef HIPHOP_ENABLE_WASI

WasmValueVector WasmEngine::nonWasiAssemblyScriptAbort(WasmValueVector params)
{
    const char *msg = WTF16ToCString(params[0]);
    const char *filename = WTF16ToCString(params[1]);
    int32_t lineNumber = params[2].of.i32;
    int32_t columnNumber = params[3].of.i32;

    // Copy format from WASI abort()
    std::cerr << "abort: " << msg << " in " << filename << "(" << lineNumber
        << ":" << columnNumber << ") - WASI is disabled" << std::endl;

    fStarted = false;

    return {};
}

#endif // HIPHOP_ENABLE_WASI
