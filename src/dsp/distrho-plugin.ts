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

// Everything contained in this file attempts to mimic the C++ plugin interfaces
// Low-level interactions with native container are strictly handled by index.ts

import { dpf_get_sample_rate } from './index'

export default namespace DISTRHO {

    // There is no support for virtual methods in AssemblyScript. Methods that
    // are implemented by the plugin author are declared in PluginInterface
    // and methods provided by the native container implemented in Plugin class.
    // C++ DISTRHO::Plugin = AS DISTRHO.Plugin + AS DISTRHO.PluginInterface

    export interface PluginInterface {

        // const char* Plugin::getLabel()
        getLabel(): string

        // const char* Plugin::getMaker()
        getMaker(): string

        // const char* Plugin::getLicense()
        getLicense(): string

        // uint32_t Plugin::getVersion()
        getVersion(): u32

        // int64_t Plugin::getUniqueId()
        getUniqueId(): i64
        
        // void Plugin::initParameter(uint32_t index, Parameter& parameter)
        initParameter(index: u32, parameter: Parameter): void

        // float Plugin::getParameterValue(uint32_t index)
        getParameterValue(index: u32): f32

        // void Plugin::setParameterValue(uint32_t index, float value)
        setParameterValue(index: u32, value: f32): void

        // void Plugin::activate()
        activate(): void

        // void Plugin::deactivate()
        deactivate(): void

        // void Plugin::run(const float** inputs, float** outputs, uint32_t frames)
        run(inputs: Float32Array[], outputs: Float32Array[]): void

    }

    export class Plugin {
        
        // double Plugin::getSampleRate();
        getSampleRate(): f32 {
            return dpf_get_sample_rate()
        }

    }

    // struct DISTRHO::Parameter
    export class Parameter {

        name: string = ''
        ranges: ParameterRanges = new ParameterRanges

    }

    // struct DISTRHO::ParameterRanges
    export class ParameterRanges {

        def: f32
        min: f32
        max: f32

    }

    // These are implemented in DistrhoUtils.cpp and are useful for calling from
    // Plugin methods getVersion() and getUniqueId()

    export function d_version(major: u8, minor: u8, micro: u8): u32 {
        return (<u32>major << 16) | (<u32>minor << 8) | (<u32>micro << 0)
    }

    export function d_cconst(a: u8, b: u8, c: u8, d: u8): i64 {
        return (<i64>a << 24) | (<i64>b << 16) | (<i64>c << 8) | (<i64>d << 0)
    }

    // This is a variation of d_cconst not found in C++, suitable for use in AS
    // because unlikely C++ writing 'A' denotes a string and not a numeric char.

    export function d_sconst(s: string): i64 {
        return d_cconst(<u8>s.charCodeAt(0), <u8>s.charCodeAt(1), 
                        <u8>s.charCodeAt(2), <u8>s.charCodeAt(3))
    }

}
