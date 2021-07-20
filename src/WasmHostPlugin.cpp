/*
 * Apices - Audio Plugins In C++ & ES6
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

#include "WasmHostPlugin.hpp"

#include "Platform.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

/**
 * FIXME: is it possible to select exports by name using the C API like in Rust?
 * 
 * Selecting by numeric index is not safe because indexes are likely to change
 * and built-in exports like "memory" are not guaranteed to always appear last.
 * 
 * Ditto for native symbols imported by the WebAssembly module.
 */

enum ExportIndex {
    GET_LABEL,
    GET_MAKER,
    GET_LICENSE,
    NUM_INPUTS,
    NUM_OUTPUTS,
    INPUT_BLOCK,
    OUTPUT_BLOCK,
    RUN,
    MEMORY
};

#define own

static own wasm_functype_t* wasm_functype_new_4_0(own wasm_valtype_t* p1, own wasm_valtype_t* p2,
                                                    own wasm_valtype_t* p3, own wasm_valtype_t* p4);
static own wasm_trap_t* wasm_abort(const wasm_val_vec_t* args, wasm_val_vec_t* results);
static own wasm_trap_t* wasm_get_sample_rate(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results);

WasmHostPlugin::WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
    : Plugin(parameterCount, programCount, stateCount)
    , fWasmEngine(0)
    , fWasmStore(0)
    , fWasmInstance(0)
    , fWasmModule(0)
{
    String path = platform::getResourcePath() + "/dsp/main.wasm";
    FILE* file = fopen(path, "rb");

    if (file == 0) {
        APX_LOG_STDERR_COLOR("Error opening Wasm module");
        return;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    wasm_byte_vec_t fileBytes;
    wasm_byte_vec_new_uninitialized(&fileBytes, fileSize);
    
    if (fread(fileBytes.data, fileSize, 1, file) != 1) {
        fclose(file);
        APX_LOG_STDERR_COLOR("Error loading Wasm module");
        return;
    }

    fclose(file);

    /*wasm_config_t* config = wasm_config_new();
    wasmer_features_t* features = wasmer_features_new();
    wasmer_features_multi_value(features, true);
    wasm_config_set_features(config, features);
    fWasmEngine = wasm_engine_new_with_config(config);*/

    fWasmEngine = wasm_engine_new();
    fWasmStore = wasm_store_new(fWasmEngine);
    fWasmModule = wasm_module_new(fWasmStore, &fileBytes); // compile

    if (!fWasmModule) {
        APX_LOG_STDERR_COLOR("Error compiling Wasm module");
        return;
    }

    wasm_byte_vec_delete(&fileBytes);

    // Allow Wasm to call some native functions

    wasm_extern_vec_t imports;
    wasm_extern_vec_new_uninitialized(&imports, 2);
    wasm_functype_t *funcType;
    wasm_func_t *func;

    funcType = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                     wasm_valtype_new_i32(), wasm_valtype_new_i32());
    func = wasm_func_new(fWasmStore, funcType, wasm_abort);
    wasm_functype_delete(funcType);
    imports.data[0] = wasm_func_as_extern(func);

    funcType = wasm_functype_new_0_1(wasm_valtype_new_f32());
    func = wasm_func_new_with_env(fWasmStore, funcType, wasm_get_sample_rate, this, 0);
    wasm_functype_delete(funcType);
    imports.data[1] = wasm_func_as_extern(func);

    wasm_trap_t* traps = 0;

    fWasmInstance = wasm_instance_new(fWasmStore, fWasmModule, &imports, &traps);

    if (!fWasmInstance) {
        APX_LOG_STDERR_COLOR("Error instantiating Wasm module");
        return;
    }

    wasm_instance_exports(fWasmInstance, &fWasmExports);

    if (fWasmExports.size == 0) {
        APX_LOG_STDERR_COLOR("Error accessing Wasm exports");
        return;
    }

    wasm_memory_t* memory = wasm_extern_as_memory(fWasmExports.data[ExportIndex::MEMORY]);
    fWasmMemoryBytes = wasm_memory_data(memory);

    wasm_val_t blockPtr;
    wasm_global_t* g;

    g = wasm_extern_as_global(fWasmExports.data[ExportIndex::INPUT_BLOCK]);
    wasm_global_get(g, &blockPtr);
    fInputBlock = reinterpret_cast<float32_t*>(&fWasmMemoryBytes[blockPtr.of.i32]);

    g = wasm_extern_as_global(fWasmExports.data[ExportIndex::OUTPUT_BLOCK]);
    wasm_global_get(g, &blockPtr);
    fOutputBlock = reinterpret_cast<float32_t*>(&fWasmMemoryBytes[blockPtr.of.i32]);

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
    wasm_func_t* func = wasm_extern_as_func(fWasmExports.data[ExportIndex::GET_LABEL]);
    wasm_val_t args[0] = {};
    wasm_val_t res[1] = { WASM_INIT_VAL };
    wasm_val_vec_t argsArray = WASM_ARRAY_VEC(args);
    wasm_val_vec_t resArray = WASM_ARRAY_VEC(res);

    wasm_trap_t* trap = wasm_func_call(func, &argsArray, &resArray);

    if (trap != 0) {
        APX_LOG_STDERR_COLOR("Error calling Wasm function");
        return 0;
    }

    const char *s = &fWasmMemoryBytes[res[0].of.i32];

    return s;
}

const char* WasmHostPlugin::getMaker() const
{
    wasm_func_t* func = wasm_extern_as_func(fWasmExports.data[ExportIndex::GET_MAKER]);
    wasm_val_t args[0] = {};
    wasm_val_t res[1] = { WASM_INIT_VAL };
    wasm_val_vec_t argsArray = WASM_ARRAY_VEC(args);
    wasm_val_vec_t resArray = WASM_ARRAY_VEC(res);

    wasm_trap_t* trap = wasm_func_call(func, &argsArray, &resArray);

    if (trap != 0) {
        APX_LOG_STDERR_COLOR("Error calling Wasm function");
        return 0;
    }

    const char *s = &fWasmMemoryBytes[res[0].of.i32];

    return s;
}

const char* WasmHostPlugin::getLicense() const
{
    wasm_func_t* func = wasm_extern_as_func(fWasmExports.data[ExportIndex::GET_LICENSE]);
    wasm_val_t args[0] = {};
    wasm_val_t res[1] = { WASM_INIT_VAL };
    wasm_val_vec_t argsArray = WASM_ARRAY_VEC(args);
    wasm_val_vec_t resArray = WASM_ARRAY_VEC(res);

    wasm_trap_t* trap = wasm_func_call(func, &argsArray, &resArray);

    if (trap != 0) {
        APX_LOG_STDERR_COLOR("Error calling Wasm function");
        return 0;
    }

    const char *s = &fWasmMemoryBytes[res[0].of.i32];

    return s;
}

uint32_t WasmHostPlugin::getVersion() const
{
    return 1;   // FIXME
}

int64_t WasmHostPlugin::getUniqueId() const
{
    return 1;   // FIXME d_cconst('D', 'P', 'w', 'g');
}

void WasmHostPlugin::initParameter(uint32_t index, Parameter& parameter)
{
    // FIXME
    (void)index;
    (void)parameter;
}

float WasmHostPlugin::getParameterValue(uint32_t index) const
{
    (void)index;
    return 0;   // FIXME
}

void WasmHostPlugin::setParameterValue(uint32_t index, float value)
{
    // FIXME
    (void)index;
    (void)value;
}

#if (DISTRHO_PLUGIN_WANT_STATE == 1)

void WasmHostPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    // FIXME
    (void)index;
    (void)stateKey;
    (void)defaultStateValue;
}

void WasmHostPlugin::setState(const char* key, const char* value)
{
    // FIXME
    (void)key;
    (void)value;
}

#if (DISTRHO_PLUGIN_WANT_FULL_STATE == 1)

String WasmHostPlugin::getState(const char* key) const
{
    // FIXME
    (void)key;
    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE == 1

#endif // DISTRHO_PLUGIN_WANT_STATE == 1

void WasmHostPlugin::activate()
{
    
}

void WasmHostPlugin::deactivate()
{
    // Empty implementation
}

void WasmHostPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    for (int i = 0; i < DISTRHO_PLUGIN_NUM_INPUTS; i++) {
        memcpy(fInputBlock + i * frames, inputs[i], frames * 4);
    }

    wasm_func_t* func = wasm_extern_as_func(fWasmExports.data[ExportIndex::RUN]);
    wasm_val_t args[1] = { WASM_I32_VAL(static_cast<int32_t>(frames)) };
    wasm_val_t res[0] = {};
    wasm_val_vec_t argsArray = WASM_ARRAY_VEC(args);
    wasm_val_vec_t resArray = WASM_ARRAY_VEC(res);

    wasm_trap_t* trap = wasm_func_call(func, &argsArray, &resArray);

    if (trap != 0) {
        APX_LOG_STDERR_COLOR("Error calling Wasm function");
        return;
    }

    for (int i = 0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; i++) {
        memcpy(outputs[i], fOutputBlock + i * frames, frames * 4);
    }
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

// Boilerplate for initializing Wasm modules compiled from AssemblyScript
static own wasm_trap_t* wasm_abort(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    // TODO - parse arguments and print them
    (void)args;
    (void)results;
    APX_LOG_STDERR_COLOR("AssemblyScript abort() called");
    return 0;
}

static own wasm_trap_t* wasm_get_sample_rate(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    (void)args;
    WasmHostPlugin* p = static_cast<WasmHostPlugin *>(env);
    wasm_val_t res[1] = { WASM_F32_VAL(static_cast<float32_t>(p->getSampleRate())) };
    wasm_val_vec_new(results, 1, res);
    return 0;
}
