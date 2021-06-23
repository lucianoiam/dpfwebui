R"WEBUI_JS(
/*
 * dpf-webui
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
