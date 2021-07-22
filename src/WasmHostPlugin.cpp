/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sstream>

#include "WasmHostPlugin.hpp"

#include "Platform.hpp"
#include "macro.h"

// Macros for reducing Wasmer C interface noise

#define own
#define WASM_DECLARE_NATIVE_FUNC(func) static own wasm_trap_t* func(void *env, const wasm_val_vec_t* args, wasm_val_vec_t* results);
#define WASM_MEMORY_CSTR(wptr) static_cast<char *>(&fWasmMemoryBytes[wptr.of.i32])
#define WASM_GLOBAL_BY_INDEX(idx) wasm_extern_as_global(fWasmExports.data[ExportIndex::idx])
#define WASM_GLOBAL_GET_BY_INDEX(idx,pval) wasm_global_get(WASM_GLOBAL_BY_INDEX(idx), pval)
#define WASM_FUNC_BY_INDEX(idx) wasm_extern_as_func(fWasmExports.data[ExportIndex::idx])
#define WASM_FUNC_CALL_BY_INDEX(idx,args,res) wasm_func_call(WASM_FUNC_BY_INDEX(idx), args, res)
#define WASM_LOG_FUNC_CALL_ERROR() HIPHAP_LOG_STDERR_COLOR("Error calling Wasm function")
#define WASM_DEFINE_ARGS_VAL_VEC_1(var,arg0) wasm_val_t var[1] = { arg0 }; \
                                             wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);
#define WASM_DEFINE_ARGS_VAL_VEC_2(var,arg0,arg1) wasm_val_t var[2] = { arg0, arg1 }; \
                                                  wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);
#define WASM_DEFINE_RES_VAL_VEC_1(var) wasm_val_t var[1] = { WASM_INIT_VAL }; \
                                       wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);

USE_NAMESPACE_DISTRHO

/**
 * FIXME: is it possible to select exports by name using the C API like in Rust?
 * Selecting by numeric index is not safe because indexes are likely to change.
 */
enum ExportIndex {
// Native re-exports
    _IGNORED_1,
// Plugin interface
    GET_LABEL,
    GET_MAKER,
    GET_LICENSE,
    GET_VERSION,
    GET_UNIQUE_ID,
    INIT_PARAMETER,
    GET_PARAMETER_VALUE,
    SET_PARAMETER_VALUE,
    ACTIVATE,
    DEACTIVATE,
    RUN,
// Globals for run()
    NUM_INPUTS,
    NUM_OUTPUTS,
    INPUT_BLOCK,
    OUTPUT_BLOCK,
// Spare variables
    RW_INT_1,
    RW_INT_2,
    RW_INT_3,
    RW_INT_4,
    RW_FLOAT_1,
    RW_FLOAT_2,
    RW_FLOAT_3,
    RW_FLOAT_4,
    RO_STRING_1,
    RO_STRING_2,
    RO_STRING_3,
    RO_STRING_4,
    RW_STRING_1,
// Built-ins
    MEMORY,
// Helpers
    ENCODE_STRING,
    _LAST_EXPORT
};

// FIXME: ditto for native symbols imported by the WebAssembly module
enum ImportIndex {
// Required by AssemblyScript
    ABORT,
// Plugin interface
    GET_SAMPLE_RATE,
    _LAST_IMPORT
};

static wasm_val_t     empty_val_array[0] = {};
static wasm_val_vec_t empty_val_vec = WASM_ARRAY_VEC(empty_val_array);

static own wasm_functype_t* wasm_functype_new_4_0(own wasm_valtype_t* p1, own wasm_valtype_t* p2,
                                                    own wasm_valtype_t* p3, own wasm_valtype_t* p4);

WASM_DECLARE_NATIVE_FUNC(ascript_abort)
WASM_DECLARE_NATIVE_FUNC(dpf_get_sample_rate)

WasmHostPlugin::WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
    : Plugin(parameterCount, programCount, stateCount)
    , fWasmEngine(0)
    , fWasmStore(0)
    , fWasmInstance(0)
    , fWasmModule(0)
{
    // -------------------------------------------------------------------------
    // Load binary module file

    String path = platform::getResourcePath() + "/dsp/main.wasm";
    FILE* file = fopen(path, "rb");

    if (file == 0) {
        HIPHAP_LOG_STDERR_COLOR("Error opening Wasm module");
        return;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    wasm_byte_vec_t fileBytes;
    wasm_byte_vec_new_uninitialized(&fileBytes, fileSize);
    
    if (fread(fileBytes.data, fileSize, 1, file) != 1) {
        fclose(file);
        HIPHAP_LOG_STDERR_COLOR("Error loading Wasm module");
        return;
    }

    fclose(file);

    // -------------------------------------------------------------------------
    // Initialize Wasmer environment

    fWasmEngine = wasm_engine_new();
    fWasmStore = wasm_store_new(fWasmEngine);
    fWasmModule = wasm_module_new(fWasmStore, &fileBytes);
    
    wasm_byte_vec_delete(&fileBytes);

    if (fWasmModule == 0) {
        HIPHAP_LOG_STDERR_COLOR("Error compiling Wasm module");
        return;
    }

    // -------------------------------------------------------------------------
    // Expose some native functions to Wasm

    wasm_extern_vec_t imports;
    wasm_extern_vec_new_uninitialized(&imports, ImportIndex::_LAST_IMPORT);
    wasm_functype_t *funcType;
    wasm_func_t *func;

    funcType = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                     wasm_valtype_new_i32(), wasm_valtype_new_i32());
    func = wasm_func_new_with_env(fWasmStore, funcType, ascript_abort, this, 0);
    wasm_functype_delete(funcType);
    imports.data[ImportIndex::ABORT] = wasm_func_as_extern(func);

    funcType = wasm_functype_new_0_1(wasm_valtype_new_f32());
    func = wasm_func_new_with_env(fWasmStore, funcType, dpf_get_sample_rate, this, 0);
    wasm_functype_delete(funcType);
    imports.data[ImportIndex::GET_SAMPLE_RATE] = wasm_func_as_extern(func);

    // -------------------------------------------------------------------------
    // Initialize Wasm module

    wasm_trap_t* traps = 0;
    fWasmInstance = wasm_instance_new(fWasmStore, fWasmModule, &imports, &traps);

    if (fWasmInstance == 0) {
        HIPHAP_LOG_STDERR_COLOR("Error instantiating Wasm module");
        return;
    }

    // -------------------------------------------------------------------------
    // Get pointers to shared memory areas

    wasm_instance_exports(fWasmInstance, &fWasmExports);

    if (fWasmExports.size == 0) {
        HIPHAP_LOG_STDERR_COLOR("Error accessing Wasm exports");
        return;
    }

    wasm_memory_t* memory = wasm_extern_as_memory(fWasmExports.data[ExportIndex::MEMORY]);
    fWasmMemoryBytes = wasm_memory_data(memory);

    wasm_val_t blockPtr;
    wasm_global_t* g;

    WASM_GLOBAL_GET_BY_INDEX(INPUT_BLOCK, &blockPtr);
    fInputBlock = reinterpret_cast<float32_t*>(&fWasmMemoryBytes[blockPtr.of.i32]);

    WASM_GLOBAL_GET_BY_INDEX(OUTPUT_BLOCK, &blockPtr);
    fOutputBlock = reinterpret_cast<float32_t*>(&fWasmMemoryBytes[blockPtr.of.i32]);

    // -------------------------------------------------------------------------
    // Initialize module globals

    g = wasm_extern_as_global(fWasmExports.data[ExportIndex::NUM_INPUTS]);
    wasm_val_t numInputs WASM_I32_VAL(static_cast<int32_t>(DISTRHO_PLUGIN_NUM_INPUTS));
    wasm_global_set(g, &numInputs);

    g = wasm_extern_as_global(fWasmExports.data[ExportIndex::NUM_OUTPUTS]);
    wasm_val_t numOutputs WASM_I32_VAL(static_cast<int32_t>(DISTRHO_PLUGIN_NUM_OUTPUTS));
    wasm_global_set(g, &numOutputs);
}

WasmHostPlugin::~WasmHostPlugin()
{
    wasm_extern_vec_delete(&fWasmExports);
    wasm_module_delete(fWasmModule);
    wasm_instance_delete(fWasmInstance);
    wasm_store_delete(fWasmStore);
    wasm_engine_delete(fWasmEngine);
}

const char* WasmHostPlugin::getLabel() const
{
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_LABEL, &empty_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return WASM_MEMORY_CSTR(res[0]);
}

const char* WasmHostPlugin::getMaker() const
{
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_MAKER, &empty_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return WASM_MEMORY_CSTR(res[0]);
}

const char* WasmHostPlugin::getLicense() const
{
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_LICENSE, &empty_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return WASM_MEMORY_CSTR(res[0]);
}

uint32_t WasmHostPlugin::getVersion() const
{
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_VERSION, &empty_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return static_cast<uint32_t>(res[0].of.i32);
}

int64_t WasmHostPlugin::getUniqueId() const
{
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_UNIQUE_ID, &empty_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return static_cast<int64_t>(res[0].of.i64);
}

void WasmHostPlugin::initParameter(uint32_t index, Parameter& parameter)
{
    // FIXME - this should be configured using AssemblyScript
    parameter.hints = kParameterIsAutomable;


    WASM_DEFINE_ARGS_VAL_VEC_1(args, WASM_I32_VAL(static_cast<int32_t>(index)));

    if (WASM_FUNC_CALL_BY_INDEX(INIT_PARAMETER, &args_val_vec, &empty_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return;
    }

    wasm_val_t res;

    WASM_GLOBAL_GET_BY_INDEX(RO_STRING_1, &res);
    parameter.name = String(WASM_MEMORY_CSTR(res));

    WASM_GLOBAL_GET_BY_INDEX(RW_FLOAT_1, &res);
    parameter.ranges.def = res.of.f32;

    WASM_GLOBAL_GET_BY_INDEX(RW_FLOAT_2, &res);
    parameter.ranges.min = res.of.f32;

    WASM_GLOBAL_GET_BY_INDEX(RW_FLOAT_3, &res);
    parameter.ranges.max = res.of.f32;
}

float WasmHostPlugin::getParameterValue(uint32_t index) const
{
    WASM_DEFINE_ARGS_VAL_VEC_1(args, WASM_I32_VAL(static_cast<int32_t>(index)));
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(GET_PARAMETER_VALUE, &args_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return res[0].of.f32;
}

void WasmHostPlugin::setParameterValue(uint32_t index, float value)
{
    WASM_DEFINE_ARGS_VAL_VEC_2(args, WASM_I32_VAL(static_cast<int32_t>(index)),
                                        WASM_F32_VAL(static_cast<float32_t>(value)));

    if (WASM_FUNC_CALL_BY_INDEX(SET_PARAMETER_VALUE, &args_val_vec, &empty_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
    }
}

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

void WasmHostPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    // TODO
    (void)index;
    (void)stateKey;
    (void)defaultStateValue;
}

void WasmHostPlugin::setState(const char* key, const char* value)
{
    // TODO
    (void)key;
    (void)value;
}

#if (DISTRHO_PLUGIN_WANT_FULL_STATE == 1)

String WasmHostPlugin::getState(const char* key) const
{
    // TODO
    (void)key;
    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE == 1

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

void WasmHostPlugin::activate()
{
    if (WASM_FUNC_CALL_BY_INDEX(ACTIVATE, &empty_val_vec, &empty_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
    }
}

void WasmHostPlugin::deactivate()
{
    if (WASM_FUNC_CALL_BY_INDEX(DEACTIVATE, &empty_val_vec, &empty_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
    }
}

void WasmHostPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    for (int i = 0; i < DISTRHO_PLUGIN_NUM_INPUTS; i++) {
        memcpy(fInputBlock + i * frames, inputs[i], frames * 4);
    }

    WASM_DEFINE_ARGS_VAL_VEC_1(args, WASM_I32_VAL(static_cast<int32_t>(frames)));

    if (WASM_FUNC_CALL_BY_INDEX(RUN, &args_val_vec, &empty_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return;
    }

    for (int i = 0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; i++) {
        memcpy(outputs[i], fOutputBlock + i * frames, frames * 4);
    }
}

const char* WasmHostPlugin::encodeString(int32_t wasmStringPtr)
{
    if (wasmStringPtr == 0) {
        return "(null)";
    }

    WASM_DEFINE_ARGS_VAL_VEC_1(args, WASM_I32_VAL(static_cast<int32_t>(wasmStringPtr)));
    WASM_DEFINE_RES_VAL_VEC_1(res);

    if (WASM_FUNC_CALL_BY_INDEX(ENCODE_STRING, &args_val_vec, &res_val_vec) != 0) {
        WASM_LOG_FUNC_CALL_ERROR();
        return 0;
    }

    return WASM_MEMORY_CSTR(res[0]);
}

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

// Required by AssemblyScript
static own wasm_trap_t* ascript_abort(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    (void)results;

    WasmHostPlugin* p = static_cast<WasmHostPlugin *>(env);

    const char *msg = p->encodeString(args->data[0].of.i32);
    const char *filename = p->encodeString(args->data[1].of.i32);
    int32_t lineNumber = args->data[2].of.i32;
    int32_t columnNumber = args->data[3].of.i32;

    std::stringstream ss;
    ss << "AssemblyScript abort() called - msg: " << msg << ", filename: " << filename
        << ", lineNumber: " << lineNumber << ", columnNumber: " << columnNumber;

    HIPHAP_LOG_STDERR_COLOR(ss.str().c_str());

    return 0;
}

static own wasm_trap_t* dpf_get_sample_rate(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    (void)args;
    WasmHostPlugin* p = static_cast<WasmHostPlugin *>(env);
    float32_t value = static_cast<float32_t>(p->getSampleRate());
    wasm_val_t res[1] = { WASM_F32_VAL(value) };
    wasm_val_vec_new(results, 1, res);
    return 0;
}
