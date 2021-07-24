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

import "wasi"
import DISTRHO from './distrho-plugin'
import PluginImpl from './plugin'

const pluginInstance = new PluginImpl

// These are external functions implemented by the native container. They are
// declared here instead of the caller module (distrho-plugin.ts) to keep all
// interfaces to the native container in a single place (index.ts) and also to
// make sure all declared functions appear listed in the module imports table.

declare function dpf_get_sample_rate(): f32

export { dpf_get_sample_rate }

// Keep dpf_get_label(), dpf_get_maker() and dpf_get_license() as function
// exports. They could be replaced with globals initialized to these function
// return values for a simpler implementation, but maybe in the future index.ts
// gets automatically injected into the Wasm VM (just like done with ui.js for
// the web view) and plugin implementations moved to "shared libraries". Under
// such scheme the guarantee that the global pluginInstance variable is already
// initialized at this point no longer holds.

export function dpf_get_label(): ArrayBuffer {
    return c_string(pluginInstance.getLabel())
}

export function dpf_get_maker(): ArrayBuffer {
    return c_string(pluginInstance.getMaker())
}

export function dpf_get_license(): ArrayBuffer {
    return c_string(pluginInstance.getLicense())
}

export function dpf_get_version(): u32 {
    return pluginInstance.getVersion()
}

export function dpf_get_unique_id(): i64 {
    return pluginInstance.getUniqueId()
}

export function dpf_init_parameter(index: u32): void {
    const parameter = new DISTRHO.Parameter
    pluginInstance.initParameter(index, parameter)
    // See explanation below for the odd value return convention
    rw_int_1 = parameter.hints
    ro_string_1 = c_string(parameter.name)
    rw_float_1 = parameter.ranges.def
    rw_float_2 = parameter.ranges.min
    rw_float_3 = parameter.ranges.max
}

export function dpf_get_parameter_value(index: u32): f32 {
    return pluginInstance.getParameterValue(index)
}

export function dpf_set_parameter_value(index: u32, value: f32): void {
    pluginInstance.setParameterValue(index, value)
}

export function dpf_activate(): void {
    pluginInstance.activate()
}

export function dpf_deactivate(): void {
    pluginInstance.deactivate()
}

export function dpf_run(frames: u32): void {
    const inputs: Array<Float32Array> = []

    for (let i = 0; i < num_inputs; i++) {
        inputs.push(Float32Array.wrap(input_block, i * frames * 4, frames))
    }

    const outputs: Array<Float32Array> = []

    for (let i = 0; i < num_outputs; i++) {
        outputs.push(Float32Array.wrap(output_block, i * frames * 4, frames))
    }

    pluginInstance.run(inputs, outputs)
}

// Number of inputs or outputs does not change during runtime so it makes sense
// to init both once instead of passing them as arguments on every call to run()

export let num_inputs: i32
export let num_outputs: i32

// Using exported globals instead of passing buffer arguments to run() allows
// for a simpler implementation by avoiding Wasm memory alloc on the host side.
// Block size should not exceed 64Kb, or 16384 frames of 32-bit float samples.

const MAX_PROCESS_BLOCK_SIZE = 65536

export let input_block = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)
export let output_block = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)

// TypedArray exports needed by the JS loader

export let input_block_float32 = Float32Array.wrap(input_block)
export let output_block_float32 = Float32Array.wrap(output_block)

// AssemblyScript does not support multi-values yet. Export a couple of generic
// variables for returning complex data types like initParameter() needs.

export let rw_int_1: i32
export let rw_int_2: i32
export let rw_int_3: i32
export let rw_int_4: i32
export let rw_float_1: f32
export let rw_float_2: f32
export let rw_float_3: f32
export let rw_float_4: f32
export let ro_string_1: ArrayBuffer
export let ro_string_2: ArrayBuffer
export let ro_string_3: ArrayBuffer
export let ro_string_4: ArrayBuffer

// These are useful for passing string arguments from the native context to Wasm

const MAX_STRING = 1024

export let rw_string_1 = new ArrayBuffer(MAX_STRING)

function c_string(s: string): ArrayBuffer {
    return String.UTF8.encode(s, true)
} 
