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
 */

class DISTRHO_WebUI {

    constructor() {
        window.webviewHost.addMessageListener((args) => {
            if (args[0] == 'WebUI') {
                this[args[1]](...args.slice(2));
            } else {
                this.messageReceived(args);
            }
        });
    }

    // UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this.postMessage('WebUI', 'editParameter', index, started);
    }

    // UI::setParameterValue(uint32_t index, float value)
    setParameterValue(index, value) {
        this.postMessage('WebUI', 'setParameterValue', index, value);
    }

    // UI::setState(const char* key, const char* value)
    setState(key, value) {
        this.postMessage('WebUI', 'setState', key, value);
    }

    // UI::parameterChanged(uint32_t index, float value)
    parameterChanged(index, value) {
        // default empty implementation
    }

    // UI::stateChanged(const char* key, const char* value)
    stateChanged(key, value) {
        // default empty implementation
    }

    // ProxyWebUI::flushInitMessageQueue()
    flushInitMessageQueue() {
        this.postMessage('WebUI', 'flushInitMessageQueue');
    }

    // ProxyWebUI::webPostMessage(const ScriptValueVector& args)
    postMessage(...args) {
        window.webviewHost.postMessage(args);
    }

    // ProxyWebUI::webMessageReceived(const ScriptValueVector& args)
    messageReceived(args) {
        // default empty implementation
    }

}

)WEBUI_JS"
