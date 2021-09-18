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

class UI {

    constructor() {
        this._resolve = {};

        window.webviewHost.addMessageListener((args) => {
            if (args[0] != 'UI') {
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

    // uint UI::getWidth()
    async getWidth() {
        return this._callAndExpectReply('getWidth');
    }

    // uint UI::getHeight()
    async getHeight() {
        return this._callAndExpectReply('getHeight');
    }

    // void UI::setWidth(uint width)
    setWidth(width) {
        this._call('setWidth', width);
    }

    // void UI::setHeight(uint height)
    setHeight(height) {
        this._call('setHeight', height);
    }

    // bool UI::isResizable()
    async isResizable() {
        return this._callAndExpectReply('isResizable');
    }

    // void UI::setSize(uint width, uint height)
    setSize(width, height) {
        this._call('setSize', width, height);
    }

    // void UI::sendNote(uint8_t channel, uint8_t note, uint8_t velocity)
    sendNote(channel, note, velocity) {
        this._call('sendNote', channel, note, velocity);
    }

    // void UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this._call('editParameter', index, started);
    }

    // void UI::setParameterValue(uint32_t index, float value)
    setParameterValue(index, value) {
        this._call('setParameterValue', index, value);
    }

    // void UI::setState(const char* key, const char* value)
    setState(key, value) {
        this._call('setState', key, value);
    }

    // void UI::sizeChanged(uint width, uint height)
    sizeChanged(width, height) {
        // default empty implementation
    }

    // void UI::parameterChanged(uint32_t index, float value)
    parameterChanged(index, value) {
        // default empty implementation
    }

    // void UI::programLoaded(uint32_t index)
    programLoaded(index) {
        // default empty implementation
    }

    // void UI::stateChanged(const char* key, const char* value)
    stateChanged(key, value) {
        // default empty implementation
    }
   
    // bool Application::isStandalone()
    async isStandalone() {
        return this._callAndExpectReply('isStandalone');
    }

    // void WebHostUI::setKeyboardFocus()
    setKeyboardFocus(focus) {
        this._call('setKeyboardFocus', focus);
    }

    // void WebHostUI::openSystemWebBrowser(String& url)
    openSystemWebBrowser(url) {
        this._call('openSystemWebBrowser', url);
    }

    // uint WebHostUI::getInitWidth()
    async getInitWidth() {
        return this._callAndExpectReply('getInitWidth');
    }

    // uint WebHostUI::getInitHeight()
    async getInitHeight() {
        return this._callAndExpectReply('getInitHeight');
    }

    // void WebHostUI::flushInitMessageQueue()
    flushInitMessageQueue() {
        this._call('flushInitMessageQueue');
    }

    // void WebHostUI::webViewPostMessage(const JsValueVector& args)
    postMessage(...args) {
        window.webviewHost.postMessage(args);
    }

    // void WebHostUI::webMessageReceived(const JsValueVector& args)
    messageReceived(args) {
        // default empty implementation
    }

    // Helper for calling UI methods
    _call(method, ...args) {
        this.postMessage('UI', method, ...args)
    }

    // Helper for supporting synchronous calls using promises
    _callAndExpectReply(method, ...args) {
        if (method in this._resolve) {
            this._resolve[method][1](); // reject previous
        }
        return new Promise((resolve, reject) => {
            this._resolve[method] = [resolve, reject];
            this._call(method, ...args);
        });
    }

}
