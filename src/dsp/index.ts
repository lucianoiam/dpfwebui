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

import PluginImpl from './plugin'

const pluginInstance = new PluginImpl

// Keep getLabel(), getMaker() and getLicense() as function exports. They could
// be replaced with globals initialized with their return values for a simpler
// implementation, but maybe in the future index.ts gets automatically injected
// into the Wasm VM (just like done with ui.js for the web view) and plugin
// implementations are moved to "shared libraries". In such scheme the guarantee
// that the global 'pluginInstance' is already initialized here no longer holds.

export function apx_get_label(): ArrayBuffer {
    return String.UTF8.encode(pluginInstance.getLabel(), true)
}

export function apx_get_maker(): ArrayBuffer {
    return String.UTF8.encode(pluginInstance.getMaker(), true)
}

export function apx_get_license(): ArrayBuffer {
    return String.UTF8.encode(pluginInstance.getLicense(), true)
}

// Number of inputs or outputs does not change during runtime so it makes sense
// to init both once instead of passing them as arguments on every call to run()

export let apx_num_inputs: i32 = 0
export let apx_num_outputs: i32 = 0

// Using exported globals instead of passing buffer arguments to run() makes
// implementation easier by avoiding Wasm memory allocation on the host side.
// Block size should not exceed 64Kb, or 16384 frames of 32-bit float samples.

const MAX_PROCESS_BLOCK_SIZE = 65536

export let apx_input_block = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)
export let apx_output_block = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)

export function apx_run(frames: i32): void {
    const inputs: Array<Float32Array> = []

    for (let i = 0; i < apx_num_inputs; i++) {
        inputs.push(Float32Array.wrap(apx_input_block, i * frames * 4, frames))
    }

    const outputs: Array<Float32Array> = []

    for (let i = 0; i < apx_num_outputs; i++) {
        outputs.push(Float32Array.wrap(apx_output_block, i * frames * 4, frames))
    }

    pluginInstance.run(inputs, outputs)
}
