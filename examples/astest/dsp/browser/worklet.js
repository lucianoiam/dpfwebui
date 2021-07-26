import loader from './loader.js'
import wasm from '../build/untouched.js'

// Converts C-style strings returned by the plugin to JS.

const cStringToJs = (ptr) =>
    String.fromCharCode.apply(null, Array.from(new Uint8Array(plugin.__getArrayBuffer(ptr)))).slice(0, -1)

// Instantiate Wasm module.

const imports = {
    index: {
        _get_sample_rate() {
            return sampleRate
        }
    }
}

const module = loader.instantiateSync(wasm, imports)

const plugin = module.exports

// Init and extract plugin parameters.

const params = []
for (let i = 0; i < 127; i++) {
    plugin._init_parameter(i)

    const param = {
        // hints: plugin.rw_int_1.value,
        name: cStringToJs(plugin._ro_string_1.value),
        defaultValue: plugin._rw_float_1.value,
        minValue: plugin._rw_float_2.value,
        maxValue: plugin._rw_float_3.value
    }

    // Heuristic for the end of parameters.
    // TODO: a less fragile way to do this?
    if (!param.name) break

    params.push(param)
}

// Set number of inputs and outputs.

plugin._num_inputs.value = 0
plugin._num_outputs.value = 2

// Register plugin.

registerProcessor(
    'tone',
    class extends AudioWorkletProcessor {
        static get parameterDescriptors() {
            return params
        }

        process(inputs, outputs, parameters) {
            const input = inputs[0]
            const output = outputs[0]
            const frames = output[0].length

            // Copy parameters from WebAudio -> Wasm.

            for (let i = 0; i < params.length; i++) {
                // TODO: Handle A-rated parameters. Currently handling only K-rated.
                plugin._set_parameter_value(i, parameters[params[i].name][0])
            }

            // Copy inputs from WebAudio -> Wasm.

            if (input)
                for (let i = 0; i < plugin.num_inputs.value; i++) {
                    if (!input[i]) break
                    plugin
                        .__getFloat32ArrayView(plugin._input_block_float32.value)
                        .subarray(i * frames, (i + 1) * frames)
                        .set(input[i])
                }

            // Run dsp.

            plugin._run(frames)

            // Copy outputs from Wasm -> WebAudio.

            for (let i = 0; i < plugin.num_outputs.value; i++) {
                if (!output[i]) break
                output[i].set(
                    plugin
                        .__getFloat32ArrayView(plugin._output_block_float32.value)
                        .subarray(i * frames, (i + 1) * frames)
                )
            }

            return true
        }
    }
)
