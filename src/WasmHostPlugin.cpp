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

#include "WasmHostPlugin.hpp"

#include "wasmer.h"

#include "macro.h"

USE_NAMESPACE_DISTRHO

Plugin* DISTRHO::createPlugin()
{
    return new WasmHostPlugin;
}

WasmHostPlugin::WasmHostPlugin()
    : Plugin(1 /* parameterCount */, 0 /* programCount */, 0 /* stateCount */)
{


    APX_LOG_STDERR("START features.c");


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

    printf("Creating the config and the features...\n");
    wasm_config_t* config = wasm_config_new();

    wasmer_features_t* features = wasmer_features_new();
    wasmer_features_multi_value(features, true); // enable multi-value!
    wasm_config_set_features(config, features);

    printf("Creating the store...\n");
    wasm_engine_t* engine = wasm_engine_new_with_config(config);
    wasm_store_t* store = wasm_store_new(engine);

    printf("Compiling module...\n");
    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);

    if (!module) {
        printf("> Error compiling module!\n");

        return;
    }

    wasm_byte_vec_delete(&wasm_bytes);

    printf("Instantiating module...\n");
    wasm_extern_vec_t imports = WASM_EMPTY_VEC;
    wasm_trap_t* traps = NULL;
    wasm_instance_t* instance = wasm_instance_new(store, module, &imports,&traps);

    if (!instance) {
        printf("> Error instantiating module!\n");

        return;
    }

    printf("Retrieving exports...\n");
    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);

    if (exports.size == 0) {
        printf("> Error accessing exports!\n");

        return;
    }

    printf("Executing `swap(1, 2)`...\n");
    wasm_func_t* swap = wasm_extern_as_func(exports.data[0]);

    wasm_val_t arguments[2] = { WASM_I32_VAL(1), WASM_I64_VAL(2) };
    wasm_val_t results[2] = { WASM_INIT_VAL, WASM_INIT_VAL };
    wasm_val_vec_t arguments_as_array = WASM_ARRAY_VEC(arguments);
    wasm_val_vec_t results_as_array = WASM_ARRAY_VEC(results);

    wasm_trap_t* trap = wasm_func_call(swap, &arguments_as_array, &results_as_array);

    if (trap != NULL) {
        printf("> Failed to call `swap`.\n");

        return;
    }

    if (results[0].of.i64 != 2 || results[1].of.i32 != 1) {
        printf("> Multi-value failed.\n");

        return;
    }

    printf("Got `(2, 1)`!\n");

    wasm_extern_vec_delete(&exports);
    wasm_module_delete(module);
    wasm_instance_delete(instance);
    wasm_store_delete(store);
    wasm_engine_delete(engine);


    APX_LOG_STDERR("END features.c");


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
