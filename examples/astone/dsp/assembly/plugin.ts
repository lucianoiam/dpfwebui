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

import 'wasi'
import DISTRHO from './distrho-plugin'

const PI_2: f32 = Mathf.PI * 2

export default class AsToneExamplePlugin extends DISTRHO.Plugin implements DISTRHO.PluginInterface {

    private frequency: f32
    private phase: f32

    getLabel(): string {
        return 'ASTone'
    }

    getMaker(): string {
        return 'George Stagas, Luciano Iam'
    }

    getLicense(): string {
        return 'ISC'
    }

    getVersion(): u32 {
        return DISTRHO.d_version(1, 0, 0)
    }

    getUniqueId(): i64 {
        return DISTRHO.d_sconst('HHtg')
    }

    initParameter(index: u32, parameter: DISTRHO.Parameter): void {
        switch (index) {
            case 0:
                parameter.hints = DISTRHO.kParameterIsAutomable
                parameter.name = 'Frequency'
                parameter.ranges.def = 440.0
                parameter.ranges.min = 220.0
                parameter.ranges.max = 880.0
                this.frequency = parameter.ranges.def   // TODO: why?
                break
        }
    }

    getParameterValue(index: u32): f32 {
        switch (index) {
            case 0:
                return this.frequency
        }

        return 0
    }

    setParameterValue(index: u32, value: f32): void {
        switch (index) {
            case 0:
                this.frequency = value
        }
    }

    run(inputs: Float32Array[], outputs: Float32Array[]): void {
        const output_l = outputs[0]
        const output_r = outputs[1]
        const radiansPerSample = PI_2 * this.frequency * (1.0 / this.getSampleRate())

        let sample: f32
        let phase: f32 = this.phase

        for (let i = 0; i < output_l.length; ++i) {
            sample = Mathf.sin(phase)
            phase += radiansPerSample
            output_l[i] = output_r[i] = sample
        }

        while (phase > PI_2) phase -= PI_2

        this.phase = phase
    }

    activate(): void {
        // Empty implementation
    }

    deactivate(): void {
        // Empty implementation
    }

}
