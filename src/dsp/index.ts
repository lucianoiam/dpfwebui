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

import PluginImpl from './plugin'

const instance = new PluginImpl

// Number of inputs or outputs does not change during runtime so it makes sense
// to init both once instead of passing them as arguments on every call to run()

export let numInputs: i32 = 0
export let numOutputs: i32 = 0

// Using exported globals instead of passing buffer arguments to run() makes
// implementation easier by avoiding Wasm memory allocation on the host side.
// Block size should not exceed 64Kb, or 16384 frames of 32-bit float samples.

const MAX_PROCESS_BLOCK_SIZE = 65536

export let inputBlock = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)
export let outputBlock = new ArrayBuffer(MAX_PROCESS_BLOCK_SIZE)

// Keep getLabel(), getMaker() and getLicense() as function exports. They could
// be replaced with globals initialized with their return values for a simpler
// implementation, but maybe in the future index.ts gets automatically injected
// into the Wasm VM (just like done with ui.js for the web view) and plugin
// implementations are moved to "shared libraries". In such scheme the guarantee
// that the global 'instance' is already initialized here no longer holds.

export function getLabel(): ArrayBuffer {
    return String.UTF8.encode(instance.getLabel(), true)
}

export function getMaker(): ArrayBuffer {
    return String.UTF8.encode(instance.getMaker(), true)
}

export function getLicense(): ArrayBuffer {
    return String.UTF8.encode(instance.getLicense(), true)
}

export function run(frames: i32): void {
    const inputs: Array<Float32Array> = []

    for (let i = 0; i < numInputs; i++) {
        inputs.push(Float32Array.wrap(inputBlock, i * frames * 4, frames))
    }

    const outputs: Array<Float32Array> = []

    for (let i = 0; i < numOutputs; i++) {
        outputs.push(Float32Array.wrap(outputBlock, i * frames * 4, frames))
    }

    instance.run(inputs, outputs)
}
