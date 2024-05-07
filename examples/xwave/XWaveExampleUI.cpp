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

#include "WebUI.hpp"
#include "VisualizationData.hpp"

class XWaveExampleUI : public WebUI
{
public:
    XWaveExampleUI()
        : WebUI(640 /*width*/, 96 /*height*/, "#0B1824" /*background*/)
        , fVisData(nullptr)
    {}

    ~XWaveExampleUI()
    {
        if (fVisData != nullptr) {
             // See comment in XWaveExamplePlugin::sharedMemoryWillDisconnect()
            notifySharedMemoryWillDisconnect();
            fVisData->~VisualizationData();
            fVisData = nullptr;
        }
    }

    void sharedMemoryCreated(uint8_t* ptr) override
    {
        fVisData =  new(ptr) VisualizationData();
    }

    void uiIdle() override
    {
        WebUI::uiIdle();

        if (fVisData != nullptr) {
            fVisData->send(*this);
        }
    }

private:
    VisualizationData* fVisData;

};

UI* DISTRHO::createUI()
{
    return new XWaveExampleUI;
}
