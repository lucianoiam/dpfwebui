/*
 * Hip-Hap / High Performance Hybrid Audio Plugins
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

class DISTRHO_UI {

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

    // void UI::parameterChanged(uint32_t index, float value)
    parameterChanged(index, value) {
        // default empty implementation
    }

    // void UI::stateChanged(const char* key, const char* value)
    stateChanged(key, value) {
        // default empty implementation
    }

    // void WebHostUI::flushInitMessageQueue()
    flushInitMessageQueue() {
        this._call('flushInitMessageQueue');
    }

    // void WebHostUI::setKeyboardFocus()
    setKeyboardFocus(focus) {
        this._call('setKeyboardFocus', focus);
    }

    // uint WebHostUI::getInitWidth()
    async getInitWidth() {
        return this._callAndExpectReply('getInitWidth');
    }

    // uint WebHostUI::getInitHeight()
    async getInitHeight() {
        return this._callAndExpectReply('getInitHeight');
    }

    // bool Application::isStandalone()
    async isStandalone() {
        return this._callAndExpectReply('isStandalone');
    }

    // void WebHostUI::webPostMessage(const ScriptValueVector& args)
    postMessage(...args) {
        window.webviewHost.postMessage(args);
    }

    // void WebHostUI::webMessageReceived(const ScriptValueVector& args)
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
