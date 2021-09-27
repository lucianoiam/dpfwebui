/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "DistrhoPlugin.hpp"

#define PARAMETER_COUNT 0
#define PROGRAM_COUNT   0
#define STATE_COUNT     1

#define STATE_INDEX_FILE 0

USE_NAMESPACE_DISTRHO

class FileChooserExamplePlugin : public Plugin
{
public:
    FileChooserExamplePlugin() : Plugin(PARAMETER_COUNT, PROGRAM_COUNT, STATE_COUNT) {}
    ~FileChooserExamplePlugin() {}

    const char* getLabel() const override
    {
        return "FileChooser";
    }

    const char* getMaker() const override
    {
        return "Luciano Iam";
    }

    const char* getLicense() const override
    {
        return "ISC";
    }

    uint32_t getVersion() const override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const override
    {
        return d_cconst('H', 'H', 'f', 'p');
    }

    void initState(uint32_t index, String& stateKey, String& defaultStateValue) override
    {
        switch (index)
        {
        case STATE_INDEX_FILE:
            stateKey = "file";
            break;
        }

        defaultStateValue = "";
    }

    void setState(const char*, const char*) override
    {
        // no-op
    }

    bool isStateFile(uint32_t index) override
    {
        return index == STATE_INDEX_FILE;
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        if (outputs[0] != inputs[0]) {
            std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);
        }
    }

    void  initParameter(uint32_t, Parameter&) override { /* no-op */ }
    float getParameterValue(uint32_t) const override { /* no-op */ return 0; }
    void  setParameterValue(uint32_t, float) override { /* no-op */ }

private:
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileChooserExamplePlugin)

};

Plugin* DISTRHO::createPlugin()
{
    return new FileChooserExamplePlugin;
}
