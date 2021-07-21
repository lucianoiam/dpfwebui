import DISTRHO from './distrho-plugin'

@inline const PI2: f32 = Mathf.PI * 2

export default class AsTestPlugin extends DISTRHO.Plugin implements DISTRHO.PluginInterface {

    private phase: f32

    getLabel(): string {
        return "AsTest"
    }

    getMaker(): string {
        return "Luciano Iam"
    }

    getLicense(): string {
        return "ISC"
    }

    getVersion(): u32 {
        return DISTRHO.d_version(1, 2, 3)
    }

    getUniqueId(): i64 {
        return DISTRHO.d_sconst('HHat')
    }

    initParameter(index: u32, parameter: DISTRHO.Parameter): void {
        // Empty implementation
    }

    getParameterValue(index: u32): f32 {
        return 0
    }

    setParameterValue(index: u32, value: f32): void {
        // Empty implementation
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
        const frequency: f32 = 440.0
        const gain: f32 = 1.0

        let sample: f32
        let phase: f32 = this.phase

        const radiansPerSample = this.freqToRadians(frequency)
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
