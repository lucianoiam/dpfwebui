const fs = require('fs')

// Read Wasm module binary into base64 for it to be inserted in source code.

const base64 = fs.readFileSync(__dirname + `/build/${process.argv[2] ?? 'optimized'}.wasm`, 'base64')

// Converts Wasm base64 back to Uint8Array bytes ready to be consumed by the WebAssembly loader.

function base64ToUint8Array(base64) {
    const table = new Uint8Array(128)
    for (let i = 0; i < 64; i++) table[i < 26 ? i + 65 : i < 52 ? i + 71 : i < 62 ? i - 4 : i * 4 - 205] = i
    const n = base64.length,
        bytes = new Uint8Array((((n - (base64[n - 1] == '=') - (base64[n - 2] == '=')) * 3) / 4) | 0)
    for (let i = 0, j = 0; i < n; ) {
        const c0 = table[base64.charCodeAt(i++)],
            c1 = table[base64.charCodeAt(i++)],
            c2 = table[base64.charCodeAt(i++)],
            c3 = table[base64.charCodeAt(i++)]
        bytes[j++] = (c0 << 2) | (c1 >> 4)
        bytes[j++] = (c1 << 4) | (c2 >> 2)
        bytes[j++] = (c2 << 6) | c3
    }
    return bytes
}

// Compile JS module and output result to stdout.

const js = `export default (${base64ToUint8Array.toString()})(${JSON.stringify(base64)})`

process.stdout.write(js)
