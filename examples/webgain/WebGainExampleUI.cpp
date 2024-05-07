/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
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

#include "WebUI.hpp"

START_NAMESPACE_DISTRHO

class WebGainExampleUI : public WebUI
{
public:
    // The color argument is for painting the native window background before
    // the web content becomes ready. Matching it to the <html> background color
    // helps eliminating flicker when opening the plugin user interface.

    WebGainExampleUI()
        : WebUI(600 /*width*/, 300 /*width*/, "#d4b6ef" /*background*/, false /*load*/)
    {
        // Web view not ready yet. Calls to runScript() or any DPF methods mapped
        // by WebUI are forbidden. Mapped methods are those that have their
        // counterparts in JavaScript; they rely on message passing and ultimately
        // runScript(). Setting the parent class constructor parameter startLoading
        // to false gives a chance to inject any needed scripts here, for example:

        String js = String(
            "window.testInjectedFunction = () => {"
            "   console.log(`webgain: the device pixel ratio is ${window.devicePixelRatio}`);"
            "};"
        );
        injectScript(js);

        // Injected scripts are queued to run immediately after the web content
        // finishes loading and before any referenced <script> starts executing.
        // It is not possible to inject scripts after calling load(). If
        // startLoading==false do not forget to call load() before returning:

        load();
    }

    ~WebGainExampleUI() {}

protected:
    void onDocumentReady() override
    {
        // Called when the main document finished loading and DOM is ready.
        // It is now safe to call runScript() and mapped DPF methods.
    }

    void onMessageReceived(const Variant& payload, uintptr_t source) override
    {
        // Web view and DOM are guaranteed to be ready here.
        (void)payload;
        (void)source;
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebGainExampleUI)

};

UI* createUI()
{
    return new WebGainExampleUI;
}

END_NAMESPACE_DISTRHO
