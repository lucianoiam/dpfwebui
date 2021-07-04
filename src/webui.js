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
        this._resolve = {};

        window.webviewHost.addMessageListener((args) => {
            if (args[0] != 'WebUI') {
                this.messageReceived(args); // passthrough
                return;
            }

            const method = args[1];
            args = args.slice(2);

            if (method in this._resolve) {
                this._resolve[method][0](...args); // fulfill promise
                delete this._resolve[method];
            } else {
                this[method](...args); // call method
            }
        });
    }

    // UI::getWidth()
    async getWidth() {
        return this._callWithReply('getWidth');
    }

    // UI::getHeight()
    async getHeight() {
        return this._callWithReply('getHeight');
    }

    // UI::setWidth(uint width)
    setWidth(width) {
        this._call('setWidth', width);
    }

    // UI::setHeight(uint height)
    setHeight(height) {
        this._call('setHeight', height);
    }

    // UI::isResizable()
    async isResizable() {
        return this._callWithReply('isResizable');
    }

    // UI::setSize(uint width, uint height)
    setSize(width, height) {
        this._call('setSize', width, height);
    }

    // UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this._call('editParameter', index, started);
    }

    // UI::setParameterValue(uint32_t index, float value)
    setParameterValue(index, value) {
        this._call('setParameterValue', index, value);
    }

    // UI::setState(const char* key, const char* value)
    setState(key, value) {
        this._call('setState', key, value);
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
        this._call('flushInitMessageQueue');
    }

    // ProxyWebUI::setGrabKeyboardInput()
    setGrabKeyboardInput(forward) {
        this._call('setGrabKeyboardInput', forward);
    }

    // ProxyWebUI::getInitWidth()
    async getInitWidth() {
        return this._callWithReply('getInitWidth');
    }

    // ProxyWebUI::getInitHeight()
    async getInitHeight() {
        return this._callWithReply('getInitHeight');
    }

    // ProxyWebUI::webPostMessage(const ScriptValueVector& args)
    postMessage(...args) {
        window.webviewHost.postMessage(args);
    }

    // ProxyWebUI::webMessageReceived(const ScriptValueVector& args)
    messageReceived(args) {
        // default empty implementation
    }

    // Helper for calling UI methods
    _call(method, ...args) {
        this.postMessage('WebUI', method, ...args)
    }

    // Helper for supporting synchronous calls using promises
    _callWithReply(method, ...args) {
        if (method in this._resolve) {
            this._resolve[method][1](); // reject previous
        }
        return new Promise((resolve, reject) => {
            this._resolve[method] = [resolve, reject];
            this._call(method, ...args);
        });
    }

}

)WEBUI_JS"
