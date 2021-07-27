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

#define own
#define WASM_DECLARE_NATIVE_FUNC(func) static own wasm_trap_t* func(void *env, \
                                            const wasm_val_vec_t* args, wasm_val_vec_t* results);
#define WASM_MEMORY() wasm_memory_data(wasm_extern_as_memory(fExportsMap["memory"]))
#define WASM_MEMORY_CSTR(wptr) static_cast<char *>(&WASM_MEMORY()[wptr.of.i32]) 
#define WASM_FUNC_CALL(name,args,res) wasm_func_call(wasm_extern_as_func(fExportsMap[name]), args, res)
#define WASM_GLOBAL_GET(name,pval) wasm_global_get(wasm_extern_as_global(fExportsMap[name]), pval)
#define WASM_GLOBAL_SET(name,pval) wasm_global_set(wasm_extern_as_global(fExportsMap[name]), pval)
#define WASM_DEFINE_ARGS_VAL_VEC_1(var,arg0) wasm_val_t var[1] = { arg0 }; \
                                             wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);
#define WASM_DEFINE_ARGS_VAL_VEC_2(var,arg0,arg1) wasm_val_t var[2] = { arg0, arg1 }; \
                                                  wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);
#define WASM_DEFINE_RES_VAL_VEC_1(var) wasm_val_t var[1] = { WASM_INIT_VAL }; \
                                       wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);

USE_NAMESPACE_DISTRHO

static wasm_val_vec_t empty_val_vec = WASM_EMPTY_VEC;

#ifndef HIPHAP_ENABLE_WASI
WASM_DECLARE_NATIVE_FUNC(ascript_abort)

static own wasm_functype_t* wasm_functype_new_4_0(own wasm_valtype_t* p1, own wasm_valtype_t* p2,
                                                  own wasm_valtype_t* p3, own wasm_valtype_t* p4);
#endif

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
    memset(&fExports, 0, sizeof(fExports));
}

WasmEngine::~WasmEngine()
{
    stop();
}

void WasmEngine::start(String& modulePath, HostImportMap& hostImports) {

    // -------------------------------------------------------------------------
    // Load and initialize binary module file

    FILE* file = fopen(modulePath, "rb");

    if (file == 0) {
        throwWasmerLastError();
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    wasm_byte_vec_t fileBytes;
    wasm_byte_vec_new_uninitialized(&fileBytes, fileSize);
    
    if (fread(fileBytes.data, fileSize, 1, file) != 1) {
        wasm_byte_vec_delete(&fileBytes);
        fclose(file);
        throwWasmerLastError();
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
    bool res = wasi_get_unordered_imports(fStore, fModule, fWasiEnv, &wasiImports);

    if (!res) {
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
    bool needsWasi = false;

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
        if (!needsWasi) {
            wn = wasm_importtype_module(importTypes.data[i]);
            memcpy(name, wn->data, wn->size);
            name[wn->size] = 0;
            if (strstr(name, "wasi_") == name) { // eg, wasi_snapshot_preview1
                needsWasi = true;
            }
        }
    }

    wasm_importtype_vec_delete(&importTypes);

#ifdef HIPHAP_ENABLE_WASI
    if (!needsWasi) {
        throw std::runtime_error("WASI is enabled but module is not WASI compliant");
    }
#else
    if (needsWasi) {
        throw std::runtime_error("WASI is not enabled but module requires WASI");
    }
#endif

    // -------------------------------------------------------------------------
    // Insert host functions into imports vector
#ifndef HIPHAP_ENABLE_WASI
    wasm_functype_t* funcType = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                     wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* func = wasm_func_new_with_env(fStore, funcType, ascript_abort, this, 0);
    wasm_functype_delete(funcType);
    imports.data[importIndex["abort"]] = wasm_func_as_extern(func);
#endif

    for (HostImportMap::const_iterator it = hostImports.begin(); it != hostImports.end(); ++it) {
        //imports.data[importIndex[it->first]] = it->second;
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

    wasm_func_call(wasiStart, &empty_val_vec, &empty_val_vec);
    wasm_func_delete(wasiStart);
#endif
    // -------------------------------------------------------------------------
    // Build a map of externs indexed by name

    fExports.size = 0;
    wasm_instance_exports(fInstance, &fExports);
    wasm_exporttype_vec_t exportTypes;
    wasm_module_exports(fModule, &exportTypes);

    for (size_t i = 0; i < fExports.size; i++) {
        const wasm_name_t *wn = wasm_exporttype_name(exportTypes.data[i]);
        memcpy(name, wn->data, wn->size);
        name[wn->size] = 0;
        fExportsMap[name] = fExports.data[i];
    }

    wasm_exporttype_vec_delete(&exportTypes);

    // -------------------------------------------------------------------------
    // Startup complete

    fStarted = true;
}

void WasmEngine::stop()
{
    if (fExports.size != 0) {
        wasm_extern_vec_delete(&fExports);
        fExports.size = 0;
    }
#ifdef HIPHAP_ENABLE_WASI
    if (fWasiEnv != 0) {
        wasi_env_delete(fWasiEnv);
        fWasiEnv = 0;
    }
#endif
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
}

wasm_val_t WasmEngine::getGlobal(String& name)
{
    // TODO
    (void)name;
    wasm_val_t value;
    return value;
}

void WasmEngine::setGlobal(String& name, wasm_val_t& value)
{
    // TODO
    (void)name;
    (void)value;
}

WasmValueVector WasmEngine::callFunction(String& name, WasmValueVector& args)
{
    // TODO
    (void)name;
    (void)args;
    WasmValueVector res;
    return res;
}

void WasmEngine::throwWasmerLastError()
{
    int len = wasmer_last_error_length();
    
    if (len == 0) {
        std::runtime_error("Wasmer unknown error");
        return;
    }

    char s[len];
    wasmer_last_error_message(s, len);

    throw std::runtime_error(std::string("Wasmer error: ") + s);
}

const char* WasmEngine::readWasmString(int32_t wasmPtr)
{
    if (wasmPtr == 0) {
        return "(null)";
    }

    WASM_DEFINE_ARGS_VAL_VEC_1(args, WASM_I32_VAL(static_cast<int32_t>(wasmPtr)));
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL("_c_string", &args_val_vec, &res_val_vec) != 0) {
        throwWasmerLastError();
    }

    return WASM_MEMORY_CSTR(res[0]);
}

#ifndef HIPHAP_ENABLE_WASI

// Convenience function, Wasmer provides up to wasm_functype_new_3_0()
static own wasm_functype_t* wasm_functype_new_4_0(own wasm_valtype_t* p1, own wasm_valtype_t* p2,
                                                    own wasm_valtype_t* p3, own wasm_valtype_t* p4)
{
    wasm_valtype_t* ps[4] = {p1, p2, p3, p4};
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 4, ps);
    wasm_valtype_vec_new_empty(&results);
    return wasm_functype_new(&params, &results);
}

// Only required when running in non-WASI mode
static own wasm_trap_t* ascript_abort(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    (void)results;

    WasmEngine* p = static_cast<WasmEngine *>(env);

    const char *msg = p->readWasmString(args->data[0].of.i32);
    const char *filename = p->readWasmString(args->data[1].of.i32);
    int32_t lineNumber = args->data[2].of.i32;
    int32_t columnNumber = args->data[3].of.i32;

    std::stringstream ss;
    ss << "AssemblyScript abort() called - msg: " << msg << ", filename: " << filename
        << ", lineNumber: " << lineNumber << ", columnNumber: " << columnNumber;

    HIPHAP_LOG_STDERR_COLOR(ss.str().c_str());

    return 0;
}

#endif // HIPHAP_ENABLE_WASI
