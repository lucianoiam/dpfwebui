/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#include <atomic>

#include "extra/PluginEx.hpp"
#include "VisualizationData.hpp"

START_NAMESPACE_DISTRHO

class XWaveExamplePlugin : public PluginEx
{
public:
    XWaveExamplePlugin()
        : PluginEx(0 /*parameters*/, 0 /*programs*/, 0 /*states*/)
        , fVisData(nullptr)
    {}

    const char* getLabel() const noexcept override
    {
        return "Wave";
    }

    const char* getDescription() const noexcept override
    {
        return "Audio waveform visualizer";
    }

    const char* getMaker() const noexcept override
    {
        return "Luciano Iam, George Stagas";
    }

    const char* getLicense() const noexcept override
    {
        return "GPLv3";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('H', 'H', 'w', 'a');
    }

    // VST3
    void initAudioPort(const bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        PluginEx::initAudioPort(input, index, port);
    }

    void sharedMemoryConnected(uint8_t* ptr) override
    {
        fVisData = reinterpret_cast<VisualizationData*>(ptr);
    }

    void sharedMemoryWillDisconnect() override
    {
        if (fVisData != nullptr) {
            if (fVisDataBusy.test_and_set()) {
                // fVisDataBusy was true, wait for current render cycle to end.
                // The max wait counter only seems to be required on Windows,
                // where the wait loop can sometimes result in a deadlock.
                // Behavior reproducible on REAPER, not tested on other DAWs.
                int maxWaitMs = 1000;
                while ((maxWaitMs-- > 0) && fVisDataBusy.test_and_set()) {
                    d_msleep(1);
                }
            } else {
                // fVisDataBusy was false and now true, free to proceed.
            }
            fVisData = nullptr;
            fVisDataBusy.clear();
        }
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        if (! fVisDataBusy.test_and_set()) {
            if (fVisData != nullptr) {
                fVisData->addSamples(inputs, frames);
            }
            
            fVisDataBusy.clear();
        }

        for (int i = 0; i < 2; ++i) {
            std::memcpy(outputs[i], inputs[i], sizeof(float) * frames);
        }
    }

private:
    std::atomic_flag   fVisDataBusy;
    VisualizationData* fVisData;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XWaveExamplePlugin)

};

Plugin* createPlugin()
{
    return new XWaveExamplePlugin;
}

END_NAMESPACE_DISTRHO
