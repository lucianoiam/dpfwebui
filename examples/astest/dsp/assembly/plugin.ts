import DISTRHO from './distrho-plugin'

@inline const PI2: f32 = Mathf.PI * 2

export default class AsTestPlugin extends DISTRHO.Plugin implements DISTRHO.PluginInterface {

    private frequency: f32
    private phase: f32

    getLabel(): string {
        return 'AssemblyScript Test'
    }

    getMaker(): string {
        return 'Luciano Iam'
    }

    getLicense(): string {
        return 'ISC'
    }

    getVersion(): u32 {
        return DISTRHO.d_version(1, 2, 3)
    }

    getUniqueId(): i64 {
        return DISTRHO.d_sconst('HHat')
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

    activate(): void {
        // Empty implementation
    }

    deactivate(): void {
        // Empty implementation
    }

    run(inputs: Float32Array[], outputs: Float32Array[]): void {
        const outputl = outputs[0]
        const outputr = outputs[1]
        const gain: f32 = 1.0

        let sample: f32
        let phase: f32 = this.phase

        const radiansPerSample = this.freqToRadians(this.frequency)
        for (let i = 0; i < outputl.length; ++i) {
            sample = Mathf.sin(phase)
            phase += radiansPerSample
            outputl[i] = outputr[i] = sample * gain
        }

        while (phase > PI2) phase -= PI2

        this.phase = phase
    }

    @inline freqToRadians(frequency: f32): f32 {
        return frequency * PI2 * (1.0 / this.getSampleRate())
    }

}
