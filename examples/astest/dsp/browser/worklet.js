import wasm from '../build/untouched.js'

// AssemblyScript / Wasm memory utilities.

const SIZE_OFFSET = -4

const getArrayBuffer = (ptr) => {
    const buffer = plugin.memory.buffer
    const length = new Uint32Array(buffer)[(ptr + SIZE_OFFSET) >>> 2]
    return buffer.slice(ptr, ptr + length)
}

const getString = (ptr) => {
    const buffer = plugin.memory.buffer
    const len = new Uint32Array(buffer)[(ptr + SIZE_OFFSET) >>> 2] >>> 1
    const arr = new Uint16Array(buffer, ptr, len)
    return String.fromCharCode.apply(String, arr)
}

const getCString = (ptr) =>
    String.fromCharCode.apply(null, Array.from(new Uint8Array(getArrayBuffer(ptr)))).slice(0, -1)

// Instantiate Wasm module.

const imports = {
    env: {
        abort(msg, file, line, colm) {
            throw Error(`abort: ${getString(msg)} at ${getString(file)}:${line}:${colm}`)
        }
    },
    index: {
        dpf_get_sample_rate() {
            return sampleRate
        }
    }
}

const module = new WebAssembly.Module(wasm)
const instance = new WebAssembly.Instance(module, imports)
const plugin = instance.exports

// Init and extract plugin parameters.

const params = []
for (let i = 0; i < 127; i++) {
    plugin.dpf_init_parameter(i)

    const param = {
        // hints: plugin.rw_int_1.value,
        name: getCString(plugin.ro_string_1.value),
        defaultValue: plugin.rw_float_1.value,
        minValue: plugin.rw_float_2.value,
        maxValue: plugin.rw_float_3.value
    }

    // Heuristic for the end of parameters.
    // TODO: a less fragile way to do this?
    if (!param.name) break

    params.push(param)
}

// Set number of inputs and outputs.

plugin.num_inputs.value = 0
plugin.num_outputs.value = 2

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
                plugin.dpf_set_parameter_value(i, parameters[params[i].name][0])
            }

            // Copy inputs from WebAudio -> Wasm.

            if (input)
                for (let i = 0; i < plugin.num_inputs.value; i++) {
                    if (!input[i]) break
                    plugin_input[i].set(
                        new Float32Array(
                            plugin.memory.buffer,
                            plugin.input_block.value + i * frames * Float32Array.BYTES_PER_ELEMENT,
                            frames
                        )
                    )
                }

            // Run dsp.

            plugin.dpf_run(frames)

            // Copy outputs from Wasm -> WebAudio.

            for (let i = 0; i < plugin.num_outputs.value; i++) {
                if (!output[i]) break
                output[i].set(
                    new Float32Array(
                        plugin.memory.buffer,
                        plugin.output_block.value + i * frames * Float32Array.BYTES_PER_ELEMENT,
                        frames
                    )
                )
            }

            return true
        }
    }
)
