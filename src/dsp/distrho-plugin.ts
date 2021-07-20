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

// There is no support for virtual methods in AssemblyScript. Methods that are
// implemented by the plugin author are declared in the DISTRHO_Plugin interface
// and methods implemented by the native container placed in DISTRHO_Plugin_Base

export interface DISTRHO_Plugin {

    // const char* Plugin::getLabel()
    getLabel(): string

    // const char* Plugin::getMaker()
    getMaker(): string

    // const char* Plugin::getLicense()
    getLicense(): string

    // void Plugin::activate()
    activate(): void

    // void Plugin::deactivate()
    deactivate(): void

    // void Plugin::run(const float** inputs, float** outputs, uint32_t frames)
    run(inputs: Float32Array[], outputs: Float32Array[]): void

}

export class DISTRHO_Plugin_Base {
    
    // double Plugin::getSampleRate();
    getSampleRate(): f32 {
        return apx_get_sample_rate()
    }

}

// Declare some functions implemented by the native container

declare function apx_get_sample_rate(): f32
