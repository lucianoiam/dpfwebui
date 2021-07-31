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

// This file attempts to mimic the C++ public plugin interfaces. Low level
// interactions with the host are strictly confined to index.ts .

import { plugin_get_samplerate, plugin_write_midi_event } from './index'

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

        // void Plugin::initProgramName(uint32_t index, String& programName)
        initProgramName(index: u32, programName: StringWrapper): void

        // void Plugin::loadProgram(uint32_t index)
        loadProgram(index: u32): void

        // void Plugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
        initState(index: u32, stateKey: StringWrapper, defaultStateValue: StringWrapper): void

        // void Plugin::setState(const char* key, const char* value)
        setState(key: string, value: string): void

        // String Plugin::getState(const char* key)
        getState(key: string): string

        // void Plugin::activate()
        activate(): void

        // void Plugin::deactivate()
        deactivate(): void

        // void Plugin::run(const float** inputs, float** outputs, uint32_t frames,
        //                  const MidiEvent* midiEvents, uint32_t midiEventCount)
        run(inputs: Float32Array[], outputs: Float32Array[], midiEvents: MidiEvent[]): void

    }

    export class Plugin {
        
        // double Plugin::getSampleRate()
        getSampleRate(): f32 {
            return plugin_get_samplerate()
        }

        // bool writeMidiEvent(const MidiEvent& midiEvent)
        writeMidiEvent(midiEvent: MidiEvent): bool {
            return plugin_write_midi_event(midiEvent)
        }

    }

    // struct DISTRHO::Parameter
    export class Parameter {

        hints: u32
        name: string = ''
        ranges: ParameterRanges = new ParameterRanges

    }

    // struct DISTRHO::ParameterRanges
    export class ParameterRanges {

        def: f32
        min: f32
        max: f32

    }

    // struct DISTRHO::MidiEvent
    export class MidiEvent {

        static readonly kDataSize: u32 = 4
        
        frame: u32
        data: Uint8Array = new Uint8Array(MidiEvent.kDataSize)

    }

    // Parameter hint constants

    export const kParameterIsAutomable: u32   = 0x01
    export const kParameterIsBoolean: u32     = 0x02
    export const kParameterIsInteger: u32     = 0x04
    export const kParameterIsLogarithmic: u32 = 0x08
    export const kParameterIsOutput: u32      = 0x10
    export const kParameterIsTrigger: u32     = 0x20 | kParameterIsBoolean

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

    // For use as mutable string argument

    export class StringWrapper {

        string: string = ''

    }

}
