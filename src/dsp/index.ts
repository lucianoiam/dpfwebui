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
    return String.UTF8.encode(pluginInstance.getLabel(), true)
}

export function dpf_get_maker(): ArrayBuffer {
    return String.UTF8.encode(pluginInstance.getMaker(), true)
}

export function dpf_get_license(): ArrayBuffer {
    return String.UTF8.encode(pluginInstance.getLicense(), true)
}

export function dpf_init_parameter(index: u32): void {
    const parameter = new DISTRHO.Parameter
    pluginInstance.initParameter(index, parameter)
    ro_string_1 = String.UTF8.encode(parameter.name, true)
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

// AssemblyScript does not support multi-values yet. Export a couple of generic
// variables for returning complex data types like initParameter() needs. These
// can be also useful for passing string arguments from native to Wasm.

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
