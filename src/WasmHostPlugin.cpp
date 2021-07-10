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

#include "macro.h"

USE_NAMESPACE_DISTRHO

WasmHostPlugin::WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
    : Plugin(parameterCount, programCount, stateCount)
    , fWasmEngine(0)
    , fWasmStore(0)
    , fWasmInstance(0)
    , fWasmModule(0)
{
    // TODO: load from file
    const char *wat_string =
        "(module\n"
        "  (type $swap_t (func (param i32 i64) (result i64 i32)))\n"
        "  (func $swap (type $swap_t) (param $x i32) (param $y i64) (result i64 i32)\n"
        "    (local.get $y)\n"
        "    (local.get $x))\n"
        "  (export \"swap\" (func $swap)))";

    wasm_byte_vec_t wat;
    wasm_byte_vec_new(&wat, strlen(wat_string), wat_string);
    wasm_byte_vec_t wasm_bytes;
    wat2wasm(&wat, &wasm_bytes);
    wasm_byte_vec_delete(&wat);
    // --------

    wasm_config_t* config = wasm_config_new();
    wasmer_features_t* features = wasmer_features_new();
    wasmer_features_multi_value(features, true);
    wasm_config_set_features(config, features);

    fWasmEngine = wasm_engine_new_with_config(config);
    fWasmStore = wasm_store_new(fWasmEngine);

    // TODO: load from file
    fWasmModule = wasm_module_new(fWasmStore, &wasm_bytes); // compile

    if (!fWasmModule) {
        APX_LOG_STDERR_COLOR("Error compiling Wasm module");
        abort();
    }

    wasm_byte_vec_delete(&wasm_bytes);

    wasm_extern_vec_t imports = WASM_EMPTY_VEC;
    wasm_trap_t* traps = NULL;
    fWasmInstance = wasm_instance_new(fWasmStore, fWasmModule, &imports, &traps);

    if (!fWasmInstance) {
        APX_LOG_STDERR_COLOR("Error instantiating Wasm module");
        abort();
    }

    wasm_instance_exports(fWasmInstance, &fWasmExports);

    if (fWasmExports.size == 0) {
        APX_LOG_STDERR_COLOR("Error accessing Wasm exports");
        abort();
    }

    wasm_func_t* swap = wasm_extern_as_func(fWasmExports.data[0]);

    wasm_val_t arguments[2] = { WASM_I32_VAL(1), WASM_I64_VAL(2) };
    wasm_val_t results[2] = { WASM_INIT_VAL, WASM_INIT_VAL };
    wasm_val_vec_t arguments_as_array = WASM_ARRAY_VEC(arguments);
    wasm_val_vec_t results_as_array = WASM_ARRAY_VEC(results);

    wasm_trap_t* trap = wasm_func_call(swap, &arguments_as_array, &results_as_array);

    if (trap != NULL) {
        APX_LOG_STDERR_COLOR("Error calling Wasm function");
        abort();
    }

    if (results[0].of.i64 != 2 || results[1].of.i32 != 1) {
        printf("> Multi-value failed.\n");
        abort();
    }

    printf("Got `(2, 1)`!\n");
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
    return "FIXME";
}

const char* WasmHostPlugin::getMaker() const
{
    return "FIXME";
}

const char* WasmHostPlugin::getLicense() const
{
    return "FIXME";
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

void WasmHostPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    // FIXME
    (void)inputs;
    (void)outputs;
    (void)frames;
}
