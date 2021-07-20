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

#ifndef WASMHOSTPLUGIN_HPP
#define WASMHOSTPLUGIN_HPP

#define WASM_API_EXTERN // link to static lib on win32
#include "wasmer.h"

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class WasmHostPlugin : public Plugin
{
public:
    WasmHostPlugin(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount);
    ~WasmHostPlugin();

    const char* getLabel() const override;
    const char* getMaker() const override;
    const char* getLicense() const override;

    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;

    void  initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

#if (DISTRHO_PLUGIN_WANT_STATE == 1)
    void   initState(uint32_t index, String& stateKey, String& defaultStateValue) override;
    void   setState(const char* key, const char* value) override;
#if (DISTRHO_PLUGIN_WANT_FULL_STATE == 1)
    String getState(const char* key) const override;
#endif
#endif

    void activate() override;
    void deactivate() override;

    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:
    wasm_engine_t*    fWasmEngine;
    wasm_store_t*     fWasmStore;
    wasm_instance_t*  fWasmInstance;
    wasm_module_t*    fWasmModule;
    wasm_extern_vec_t fWasmExports;
    byte_t*           fWasmMemoryBytes;
    float32_t*        fInputBlock;
    float32_t*        fOutputBlock;

};

END_NAMESPACE_DISTRHO

#endif  // WASMHOSTPLUGIN_HPP
