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
        return "GPLv3";
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

private:
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileChooserExamplePlugin)

};

Plugin* DISTRHO::createPlugin()
{
    return new FileChooserExamplePlugin;
}
