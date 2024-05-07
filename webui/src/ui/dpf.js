/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

(new function() {

class UI {

    constructor(opt) {
        this._opt = opt || {};
        this._initMessageChannel();
    }

    // uint UI::getWidth()
    async getWidth() {
        return this.call('getWidth');
    }

    // uint UI::getHeight()
    async getHeight() {
        return this.call('getHeight');
    }

    // void UI::setWidth(uint width)
    setWidth(width) {
        this.call('setWidth', width);
    }

    // void UI::setHeight(uint height)
    setHeight(height) {
        this.call('setHeight', height);
    }

    // bool UI::isResizable()
    async isResizable() {
        return this.call('isResizable');
    }

    // void UI::setSize(uint width, uint height)
    // This method is only supported in the plugin embedded web view
    setSize(width, height) {
        if (DISTRHO.env.plugin) {
            this.call('setSize', width, height);
        }
    }

    // void UI::sendNote(uint8_t channel, uint8_t note, uint8_t velocity)
    sendNote(channel, note, velocity) {
        this.call('sendNote', channel, note, velocity);
    }

    // double UI::getSampleRate()
    async getSampleRate() {
        return this.call('getSampleRate');
    }

    // void UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this.call('editParameter', index, started);
    }

    // void UI::setParameterValue(uint32_t index, float value)
    setParameterValue(index, value) {
        this.call('setParameterValue', index, value);
    }

    // void UI::setState(const char* key, const char* value)
    setState(key, value) {
        this.call('setState', key, value);
    }

    // void UI::sizeChanged(uint width, uint height)
    sizeChanged(width, height) {}

    // void UI::parameterChanged(uint32_t index, float value)
    parameterChanged(index, value) {}

    // void UI::programLoaded(uint32_t index)
    programLoaded(index) {}

    // void UI::stateChanged(const char* key, const char* value)
    stateChanged(key, value) {}

    // void UI::sampleRateChanged(double newSampleRate)
    sampleRateChanged(newSampleRate) {}
   
    // bool ExternalWindow::isStandalone()
    async isStandalone() {
        return this.call('isStandalone');
    }

    // Non-DPF method for grabbing or releasing the keyboard focus
    // void WebViewUI::setKeyboardFocus()
    setKeyboardFocus(focus) {
        if (DISTRHO.env.plugin) {
            this.call('setKeyboardFocus', focus);
        }
    }

    // Non-DPF method for opening the default system browser
    // void WebViewUI::openSystemWebBrowser(String& url)
    openSystemWebBrowser(url) {
        if (DISTRHO.env.plugin) {
            this.call('openSystemWebBrowser', url);
        } else {
            window.open(url, '_blank');
        }
    }

    // Non-DPF method that returns the UI width in CSS px at initialization time
    // uint WebViewUI::getInitWidthCSS()
    async getInitWidthCSS() {
        return this.call('getInitWidthCSS');
    }

    // Non-DPF method that returns the UI height in CSS px at initialization time
    // uint WebViewUI::getInitHeightCSS()
    async getInitHeightCSS() {
        return this.call('getInitHeightCSS');
    }

    // Non-DPF method for sending a message to the host
    // void WebViewUI::postMessage(const Variant& payload)
    postMessage(...payload) {
        const env = DISTRHO.env;
        const socketSend = payload => {
            if (this._socket.readyState == WebSocket.OPEN) {
                let data;

                if (this._isProtocolBinary) {
                    data = BSON.serialize(payload.reduce((acc, val, idx) => {
                        acc[idx] = val;
                        return acc;
                    }, {}));
                } else {
                    data = JSON.stringify(payload);
                }

                this._socket.send(data);
            } else {
                this._log(`Cannot send message, socket state is ${this._socket.readyState}.`);
            }
        }

        if (env.plugin) {
            if (env.network) {
                socketSend(payload);
            } else {
                window.host.postMessage(payload);
            }
        } else {
            if (env.dev) {
                this._log(`Stub postMessage(${payload})`);
            } else if (env.network) {
                socketSend(payload);
            }

        }
    }

    // Non-DPF method for sending a message to all connected network clients
    broadcastMessage(...payload) {
        this.postMessage('broadcast', ...payload)
    }

    // Non-DPF callback method for receiving messages from the host
    // void WebUIBase::onMessageReceived(const Variant& payload)
    messageReceived(payload) {}

    // Non-DPF callback method that fires when the message channel is open
    messageChannelOpen() {}

    // Non-DPF callback method that fires when the message channel is closed
    messageChannelClosed() {}

    // Non-DPF callback method when memory shared with Plugin is ready to use
    // void UIEx::sharedMemoryCreated(uint8_t* ptr)
    sharedMemoryCreated() {}

    // Non-DPF method that writes to memory shared with DISTRHO::PluginEx instance
    // void UIEx::writeSharedMemory(const uint8_t* data, size_t size, size_t offset)
    writeSharedMemory(data /*Uint8Array*/, offset) {
        this.call('writeSharedMemory', this._encodeBinaryDataIfNeeded(data), offset || 0);
    }

    // Non-DPF method that returns the plugin UI public URL
    // String NetworkUI::getPublicUrl()
    async getPublicUrl() {
        if (DISTRHO.env.dev) {
            return document.location.href;
        } else {
            return this.call('getPublicUrl');
        }
    }

    // Non-DPF method to check whether the plugin is published using Zeroconf
    async isZeroconfPublished() {
        return this.call('isZeroconfPublished');
    }

    // Non-DPF method for toggling Zeroconf publish state
    setZeroconfPublished(published) {
        this.call('setZeroconfPublished', published);
    }

    // Non-DPF method for querying Zeroconf plugin instance ID (read-only)
    async getZeroconfId() {
        return this.call('getZeroconfId');
    }

     // Non-DPF method for querying Zeroconf service name
    async getZeroconfName() {
        return this.call('getZeroconfName');
    }

    // Non-DPF method for updating Zeroconf service name
    setZeroconfName(name) {
        this.call('setZeroconfName', name);
    }

    // Non-DPF method for getting approximate network latency in milliseconds
    getNetworkLatency() {
        return this._latency;
    }

    // Non-DPF callback method that fires when network latency is updated
    networkLatencyChanged(latencyMs) {}

    // Non-DPF method for for calling native UI functions
    call(funcName, ...args) {
        if (this._resolve[funcName] === undefined) {
            this._resolve[funcName] = [];
        }
        return new Promise((resolve, reject) => {
            this._resolve[funcName].push({resolve: resolve, reject: reject});
            const funcArg = this._isProtocolBinary ? this.constructor.djb2hash(funcName)
                            : funcName;
            this.postMessage(funcArg, ...args)
        });
    }

    //
    // Private methods
    //

    _initMessageChannel() {
        this._resolve = {};
        this._socket = null;
        this._latency = 0;
        this._pingSendTime = 0;
        this._callbackLookup = this;

        const env = DISTRHO.env;

        if (env.plugin) {
            if (env.network) {
                this._initSocketMessageChannel();
            } else {
                this._initNativeMessageChannel();
                // Call WebUI::ready() to let the host know the JS UI has completed
                // setting up. This causes the initialization message buffer to be
                // flushed so any messages generated while the web view was still
                // loading can be processed. Since this involves message passing, it
                // will not cause any UI methods to be triggered synchronously and
                // is safe to indirectly call from super() in subclass constructors.
                this.call('ready');
            }
        } else {
            if (env.dev) {
                setTimeout(this.messageChannelOpen.bind(this), 0);
            } else if (env.network) {
                this._initSocketMessageChannel();
            }
        }
    }

    // Initialize WebSockets-based message channel for network clients
    _initSocketMessageChannel() {
        const reconnectPeriod = 3;
        const pingPeriod = 10;

        let reconnectTimer = null;
        let pingTimer = null;

        const open = () => {
            this._socket = new WebSocket(`ws://${document.location.host}`);
            this._socket.binaryType = 'arraybuffer';

            this._socket.addEventListener('open', (_) => {
                this._log('Connected');

                if (this._isProtocolProbed) {
                    this.messageChannelOpen();
                }

                clearInterval(reconnectTimer);
                pingTimer = setInterval(this._ping.bind(this), 1000 * pingPeriod);
                this._ping();
            });

            this._socket.addEventListener('close', (_) => {
                this._log(`Reconnecting in ${reconnectPeriod} sec...`);

                this._cancelAllRequests();
                this.messageChannelClosed();

                clearInterval(pingTimer);
                clearInterval(reconnectTimer);
                reconnectTimer = setInterval(open, 1000 * reconnectPeriod);
            });

            this._socket.addEventListener('message', (ev) => {
                if (! this._isProtocolProbed) {
                    this._isProtocolProbed = true;
                    this._probeProtocol(ev.data);

                    this.messageChannelOpen();
                }

                let payload;

                if (this._isProtocolBinary) {
                    const payloadArr = BSON.deserialize(ev.data);
                    payload = Object.keys(payloadArr).map(k => payloadArr[k]);
                } else {
                    payload = JSON.parse(ev.data);
                }

                this._messageReceived(payload);
            });
        };

        open();
    }

    // Initialize native C++/JS message channel for the embedded web view
    _initNativeMessageChannel() {
        window.host.addMessageListener(this._messageReceived.bind(this));
        // Make sure subclass constructor completed before firing callback
        setTimeout(this.messageChannelOpen.bind(this), 0);
    }

    // Send a ping message to measure latency
    _ping() {
        this._pingSendTime = (new Date).getTime();
        this.call('ping');
    }

    // Compute latency when response to ping is received
    pong() {
        this._latency = ((new Date).getTime() - this._pingSendTime) / 2;
        this.networkLatencyChanged(this._latency);
        this._log(`Latency = ${this._latency}ms`);
    }

    // Handle incoming message
    _messageReceived(payload) {
        const func = this._callbackLookup[payload[0]];

        if (! func) {
            this.messageReceived(payload); // passthrough
            return;
        }
        
        const funcName = func.name;
        payload = payload.slice(1);

        if (funcName in this._resolve) {
            for (let callback of this._resolve[funcName]) {
                callback.resolve(...payload);
            }
            this._resolve[funcName] = [];
        } else {
            func.call(this, ...payload);
        }
    }

    // Encode binary data to base64 when the protocol is text-based
    _encodeBinaryDataIfNeeded(data) {
        return this._isProtocolBinary ? data : base64EncArr(data);
    }

    // Reject all pending promises on channel disconnection
    _cancelAllRequests() {
        for (let funcName in this._resolve) {
            for (let callback of this._resolve[funcName]) {
                callback.reject();
            }

            this._resolve[funcName] = [];
        }
    }

    // Detect if the message protocol is text-based or binary
    _probeProtocol(data) {
        this._isProtocolBinary = data instanceof ArrayBuffer;
        
        if (this._isProtocolBinary) {
            if (typeof(BSON) === 'undefined') {
                throw new Error('Binary socket requires BSON, make sure bson.min.js is loaded.');
            }

            // Hash all function names
            this._callbackLookup = this._getInstanceFunctions().reduce((acc, func) => {
                acc[this.constructor.djb2hash(func.name)] = func;
                return acc;
            }, {});
        }
    }

    // Optional debug messages
    _log(message) {
        if (this._opt.log) {
            console.log(`UI: ${message}`);
        }
    }

    // Binary message protocol support
    _getInstanceFunctions() {
        const func = [];
        let obj = this;

        do {
            func.push(...Object.getOwnPropertyNames(obj)
                        .map(name => this[name])
                        .filter(prop => typeof(prop) === 'function'));
        } while (obj = Object.getPrototypeOf(obj));

        return func;
    }

    // Modified version of https://gist.github.com/eplawless/52813b1d8ad9af510d85
    static djb2hash(str) {
        let len = str.length;
        let h = 5381;

        for (let i = 0; i < len; i++) {
            h = h * 33 ^ str.charCodeAt(i)
        }

        return h;
    }

}

//
// Public utility functions
//
class UIHelper {

    static bindSystemBrowser(ui, el, opt) {
        opt = opt || {};
        const url = opt.url || el.href;

        ['touchstart', 'click'].forEach((evName) => {
            el.addEventListener(evName, (ev) => {
                ui.openSystemWebBrowser(url);

                if (opt.callback instanceof Function) {
                    opt.callback();
                }

                if (ev.cancelable) {
                    ev.preventDefault();
                }
            });
        });
    }

    static enableOfflineModal(ui, opt) {
        opt = opt || {};
        opt.parent = opt.parent || document.body;

        // Monkey patch UI message channel callbacks
        const openUiCallback = ui.messageChannelOpen.bind(ui);
        const closedUiCallback = ui.messageChannelClosed.bind(ui);

        ui.messageChannelOpen = () => {
            if (ui._offlineModalTimeout) {
                clearTimeout(ui._offlineModalTimeout);
                delete ui._offlineModalTimeout;
            }

            if (ui._offlineModal) {
                opt.parent.removeChild(ui._offlineModal);
                delete ui._offlineModal;
            }

            openUiCallback();
        };

        ui.messageChannelClosed = () => {
            closedUiCallback();

            if (ui._offlineModalTimeout) {
                clearTimeout(ui._offlineModalTimeout);
            }

            ui._offlineModalTimeout = setTimeout(() => {
                delete ui._offlineModalTimeout;

                if (! ui._offlineModal) {
                    ui._offlineModal = this.getOfflineModalElement();
                    opt.parent.appendChild(ui._offlineModal);
                }
            }, 1000);
        };
    }

    static getOfflineModalElement() {
        const html =
            `<div 
                style="
                display: flex;
                justify-content: center;
                align-items: center;
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                z-index: 10;
                background: rgba(0,0,0,0.75)">
                <svg
                   xmlns="http://www.w3.org/2000/svg"
                   width="96"
                   height="96"
                   viewBox="0 0 400 400">
                  <g
                     style="fill:#fff;stroke:#fff;stroke-width:2.23000002"
                     transform="translate(183.604,196.39599)">
                    <path
                        d="m -116.9899,106.24536 31.819808,31.8198 236.310532,-236.310518 -31.81982,-31.819802 z" />
                    <circle
                        style="fill:none;stroke-width:44.59999847"
                        r="171.304"
                        cy="4"
                        cx="16" />
                  </g>
                </svg>
            </div>`;

        return document.createRange().createContextualFragment(html).firstChild;
    }

    static getNetworkDetailsModalButton(ui, opt) {
        opt = opt || {};
        opt.size = opt.size || 24;
        opt.fill = opt.fill || '#fff';

        const html =
            `<div
                style="
                width: ${opt.size}px;
                height: ${opt.size}px;">
                <a href="#" style="cursor:default;">
                    <svg
                        xmlns="http://www.w3.org/2000/svg"
                        width="${opt.size}"
                        height="${opt.size}"
                        fill="${opt.fill}"
                        viewBox="-45.5 82.5 36 36">
                        <g>
                            <path d="M-42.456,85.654v10.852h10.905V85.654H-42.456z M-34.173,93.871h-5.616v-5.578v-0.006v0.006h5.616V93.871z"/>
                            <rect x="-15.271" y="112.883" width="2.771" height="2.729"/>
                            <polygon points="-20.611,112.883 -17.828,112.883 -17.828,115.471 -23.389,115.471 -23.389,110.154 -20.707,110.154 
                                -20.707,107.449 -17.971,107.449 -17.971,102.07 -15.182,102.07 -15.182,104.715 -12.539,104.715 -12.539,107.449 -15.182,107.449 
                                -15.182,110.18 -20.611,110.18 -20.611,112.871   "/>
                            <polygon points="-20.746,101.934 -23.395,101.934 -23.395,99.199 -26.088,99.199 -26.088,91.079 -28.865,91.079 -28.865,99.199 
                                -31.603,99.199 -31.603,101.938 -28.818,101.938 -28.818,99.199 -26.174,99.199 -26.174,101.938 -28.763,101.938 -28.763,104.672 
                                -26.174,104.672 -26.174,107.412 -28.861,107.412 -28.861,115.484 -26.083,115.484 -26.083,110.055 -23.389,110.055 
                                -23.389,104.672 -20.611,104.672 -20.611,101.934 -17.965,101.934 -17.965,99.199 -20.746,99.199   "/>
                            <rect x="-15.313" y="99.199" width="2.773" height="2.734"/>
                            <rect x="-37.024" y="99.199" width="2.779" height="2.734"/>
                            <rect x="-42.456" y="99.199" width="2.783" height="2.734"/>
                            <rect x="-28.861" y="85.654" width="2.778" height="2.734"/>
                            <path d="M-42.5,104.715v10.854h10.897v-10.854H-42.5L-42.5,104.715z M-34.231,112.934h-5.621v-5.572h5.621V112.934z"/>
                            <path d="M-23.43,85.654h-0.004v10.852h10.896V85.654H-23.43z M-15.162,93.871h-5.623v-5.578v-0.006v0.006h5.623V93.871z"/>
                        </g>
                    </svg>
                </a>
            </div>`;

        const el = document.createRange().createContextualFragment(html).firstChild;
        el.id = opt.id;

        ['touchstart', 'click'].forEach((evName) => {
            el.querySelector('a').addEventListener(evName, (ev) => {
                this.showNetworkDetailsModal(ui, opt.modal);
                if (ev.cancelable) {
                    ev.preventDefault();
                }
            });
        });

        return el;
    }

    static async getNetworkDetailsElement(ui, opt) {
        opt = opt || {};
        opt.size = opt.size || 96; // px
        opt.fontSize = opt.fontSize || opt.size / 8; // px
        opt.gap = opt.gap || 50; // px

        const url = await ui.getPublicUrl();
        const zcPublished = await ui.isZeroconfPublished();
        const zcId = await ui.getZeroconfId();
        const zcName = await ui.getZeroconfName();

        const qrSvg = new QRCode({
            content: url,
            padding: 4,
            width: opt.size,
            height: opt.size
        }).svg();

        const html =
           `<div
                data-url="${url}"
                style="
                display: flex;
                align-items: center;
                justify-content: center;
                width: 100%;
                height: 100%;">
                <div
                    style="
                    display: flex;
                    flex-direction: ${opt.vertical ? 'column' : 'row'};
                    align-items: center;
                    justify-content: space-evenly;
                    gap: ${opt.gap}px;">
                    ${qrSvg}
                    <div
                        style="
                        display: flex;
                        flex-direction: column;
                        font-family: monospace;
                        font-size: ${opt.fontSize}px;
                        color: #fff;
                        gap: 1em;">
                        <div>
                            <a href="#" style="color:#fff">${url}</a>
                        </div>
                        ${zcId ? `<div>ID ${zcId}</div>` : '' }
                        <div
                            style="
                            display: flex;
                            flex-direction: row;
                            align-items: center;">
                            <span>Name</span>
                            &nbsp;
                            <input
                                style="
                                color: #fff;
                                background: #000;
                                border: solid 1px #fff;
                                padding: ${opt.fontSize / 6}px ${opt.fontSize / 3}px;
                                font-family: monospace;
                                font-size: ${opt.fontSize}px;"
                                type="text"
                                value="${zcName}">
                        </div>
                    </div>
                </div>
            </div>`;

        const el = document.createRange().createContextualFragment(html).firstChild;
        el.style.height = '100%';
        
        ['touchstart', 'click'].forEach((evName) => {
            el.querySelector('a').addEventListener(evName, (ev) => {
                ui.openSystemWebBrowser(url);
                if (ev.cancelable) {
                    ev.preventDefault();
                }
            });
        });

        const nameInput = el.querySelector('input');

        if (zcPublished) {
            nameInput.addEventListener('focus', _ => ui.setKeyboardFocus(true));
            nameInput.addEventListener('blur', _ => ui.setKeyboardFocus(false));
            nameInput.addEventListener('change', _ => {
                ui.setZeroconfName(nameInput.value);
                nameInput.blur();
            });
        } else {
            const nameBox = nameInput.parentNode;
            nameBox.parentNode.removeChild(nameBox);
        }

        return el;
    }

    static async showNetworkDetailsModal(ui, opt) {
        opt = opt || {};
        opt.parent = opt.parent || document.body;

        const html =
            `<div>
                <a
                    href="#"
                    style="
                    position: absolute;
                    top: 0;
                    right: 0;
                    width: 36x;
                    height: 36px;
                    padding-top: 8px;
                    padding-right: 8px;
                    cursor: default;">
                    <svg
                        xmlns="http://www.w3.org/2000/svg"
                        width="28"
                        height="28"
                        viewBox="0 0 24 24">
                        <g id="close">
                            <path id="x" fill="#fff" d="M18.717 6.697l-1.414-1.414-5.303 5.303-5.303-5.303-1.414 1.414 5.303 5.303-5.303 5.303 1.414 1.414 5.303-5.303 5.303 5.303 1.414-1.414-5.303-5.303z"/>
                        </g>
                    </svg>
                </a>
            </div>`;

        const el = document.createRange().createContextualFragment(html).firstChild;
        el.id = opt.id;

        el.appendChild(await this.getNetworkDetailsElement(ui));

        el.querySelectorAll('a').forEach((a) => {
            ['touchstart', 'click'].forEach((evName) => {
                a.addEventListener(evName, (ev) => {
                    opt.parent.removeChild(el);
                    if (ev.cancelable) {
                        ev.preventDefault();
                    }
                });
            });
        });

        opt.parent.appendChild(el);
    }

}

//
// Private utility functions
//
class UIHelperPrivate {

    static applyTweaks(env) {
        // Disable context menu
        window.addEventListener('contextmenu', (ev) => {
            ev.preventDefault()
        });

        // Disable print key shortcut
        window.addEventListener('keydown', (ev) => { 
            if ((ev.key == 'p') && (ev.ctrlKey || ev.metaKey)) {
                ev.preventDefault();
            }
        });

        // Disable pinch zoom. Might still be desirable for web browsers.
        const optDisablePinchZoom = env.plugin ? 'touch-action: pan-x pan-y;' : '';

        // Disable I-beam for text, disable image drag, disable selection, disable overflow.
        const applyStyle = () => {
            const css =
                `*:not(a) {
                    cursor: default;
                }
                img {
                    user-drag: none;
                    -webkit-user-drag: none;
                }
                body {
                    user-select: none;
                    -webkit-user-select: none;
                    overflow: hidden;
                    ${optDisablePinchZoom}
                }`;

            document.head.insertAdjacentHTML('beforeend', `<style>${css}</style>`);
        };

        // document.head might be null when dpf.js is injected by some web views
        if (document.head) {
            applyStyle();
        } else {
            document.addEventListener('DOMContentLoaded', applyStyle);
        }
    }

    static buildEnvObject() {
        // Determine the running environment. This information could be prepared
        // on the native side and then 1) injected into the web view, or 2)
        // injected into dpf.js before it is served (so it also works for HTTP
        // clients). But the simplicity of the client-side heuristics below
        // outweighs the complexity of the server-side solution with same
        // results in practice.
        let env = {};

        if (window.host !== undefined) {
            env.plugin = true;
            if (window.host.env) {
                Object.assign(env, window.host.env);
                delete window.host.env;
            }
        } else {
            env.plugin = false;
        }

        env.network = window.location.protocol.indexOf('http') == 0;

        const params = new URLSearchParams(window.location.search);
        if (params.get('dev') == '1') {
            env.dev = true;
        } else {
            env.dev = !env.plugin && !env.network; // ie. open file index.html
        }

        return Object.freeze(env);
    }

}

// +------------------------------------------------------------------------+ //
// |                   COPY AND PASTED VENDOR CODE BEGIN                    | //
// +------------------------------------------------------------------------+ //

//---------------------------------------------------------------------
// QRCode for JavaScript
//
// Copyright (c) 2009 Kazuhiko Arase
//
// URL: http://www.d-project.com/
//
// Licensed under the MIT license:
//   http://www.opensource.org/licenses/mit-license.php
//
// The word "QR Code" is registered trademark of 
// DENSO WAVE INCORPORATED
//   http://www.denso-wave.com/qrcode/faqpatent-e.html
//
//---------------------------------------------------------------------
function QR8bitByte(data) {
  this.mode = QRMode.MODE_8BIT_BYTE;
  this.data = data;
  this.parsedData = [];

  // Added to support UTF-8 Characters
  for (var i = 0, l = this.data.length; i < l; i++) {
    var byteArray = [];
    var code = this.data.charCodeAt(i);

    if (code > 0x10000) {
      byteArray[0] = 0xF0 | ((code & 0x1C0000) >>> 18);
      byteArray[1] = 0x80 | ((code & 0x3F000) >>> 12);
      byteArray[2] = 0x80 | ((code & 0xFC0) >>> 6);
      byteArray[3] = 0x80 | (code & 0x3F);
    } else if (code > 0x800) {
      byteArray[0] = 0xE0 | ((code & 0xF000) >>> 12);
      byteArray[1] = 0x80 | ((code & 0xFC0) >>> 6);
      byteArray[2] = 0x80 | (code & 0x3F);
    } else if (code > 0x80) {
      byteArray[0] = 0xC0 | ((code & 0x7C0) >>> 6);
      byteArray[1] = 0x80 | (code & 0x3F);
    } else {
      byteArray[0] = code;
    }

    this.parsedData.push(byteArray);
  }

  this.parsedData = Array.prototype.concat.apply([], this.parsedData);

  if (this.parsedData.length != this.data.length) {
    this.parsedData.unshift(191);
    this.parsedData.unshift(187);
    this.parsedData.unshift(239);
  }
}

QR8bitByte.prototype = {
  getLength: function (buffer) {
    return this.parsedData.length;
  },
  write: function (buffer) {
    for (var i = 0, l = this.parsedData.length; i < l; i++) {
      buffer.put(this.parsedData[i], 8);
    }
  }
};

function QRCodeModel(typeNumber, errorCorrectLevel) {
  this.typeNumber = typeNumber;
  this.errorCorrectLevel = errorCorrectLevel;
  this.modules = null;
  this.moduleCount = 0;
  this.dataCache = null;
  this.dataList = [];
}

QRCodeModel.prototype={addData:function(data){var newData=new QR8bitByte(data);this.dataList.push(newData);this.dataCache=null;},isDark:function(row,col){if(row<0||this.moduleCount<=row||col<0||this.moduleCount<=col){throw new Error(row+","+col);}
return this.modules[row][col];},getModuleCount:function(){return this.moduleCount;},make:function(){this.makeImpl(false,this.getBestMaskPattern());},makeImpl:function(test,maskPattern){this.moduleCount=this.typeNumber*4+17;this.modules=new Array(this.moduleCount);for(var row=0;row<this.moduleCount;row++){this.modules[row]=new Array(this.moduleCount);for(var col=0;col<this.moduleCount;col++){this.modules[row][col]=null;}}
this.setupPositionProbePattern(0,0);this.setupPositionProbePattern(this.moduleCount-7,0);this.setupPositionProbePattern(0,this.moduleCount-7);this.setupPositionAdjustPattern();this.setupTimingPattern();this.setupTypeInfo(test,maskPattern);if(this.typeNumber>=7){this.setupTypeNumber(test);}
if(this.dataCache==null){this.dataCache=QRCodeModel.createData(this.typeNumber,this.errorCorrectLevel,this.dataList);}
this.mapData(this.dataCache,maskPattern);},setupPositionProbePattern:function(row,col){for(var r=-1;r<=7;r++){if(row+r<=-1||this.moduleCount<=row+r)continue;for(var c=-1;c<=7;c++){if(col+c<=-1||this.moduleCount<=col+c)continue;if((0<=r&&r<=6&&(c==0||c==6))||(0<=c&&c<=6&&(r==0||r==6))||(2<=r&&r<=4&&2<=c&&c<=4)){this.modules[row+r][col+c]=true;}else{this.modules[row+r][col+c]=false;}}}},getBestMaskPattern:function(){var minLostPoint=0;var pattern=0;for(var i=0;i<8;i++){this.makeImpl(true,i);var lostPoint=QRUtil.getLostPoint(this);if(i==0||minLostPoint>lostPoint){minLostPoint=lostPoint;pattern=i;}}
return pattern;},createMovieClip:function(target_mc,instance_name,depth){var qr_mc=target_mc.createEmptyMovieClip(instance_name,depth);var cs=1;this.make();for(var row=0;row<this.modules.length;row++){var y=row*cs;for(var col=0;col<this.modules[row].length;col++){var x=col*cs;var dark=this.modules[row][col];if(dark){qr_mc.beginFill(0,100);qr_mc.moveTo(x,y);qr_mc.lineTo(x+cs,y);qr_mc.lineTo(x+cs,y+cs);qr_mc.lineTo(x,y+cs);qr_mc.endFill();}}}
return qr_mc;},setupTimingPattern:function(){for(var r=8;r<this.moduleCount-8;r++){if(this.modules[r][6]!=null){continue;}
this.modules[r][6]=(r%2==0);}
for(var c=8;c<this.moduleCount-8;c++){if(this.modules[6][c]!=null){continue;}
this.modules[6][c]=(c%2==0);}},setupPositionAdjustPattern:function(){var pos=QRUtil.getPatternPosition(this.typeNumber);for(var i=0;i<pos.length;i++){for(var j=0;j<pos.length;j++){var row=pos[i];var col=pos[j];if(this.modules[row][col]!=null){continue;}
for(var r=-2;r<=2;r++){for(var c=-2;c<=2;c++){if(r==-2||r==2||c==-2||c==2||(r==0&&c==0)){this.modules[row+r][col+c]=true;}else{this.modules[row+r][col+c]=false;}}}}}},setupTypeNumber:function(test){var bits=QRUtil.getBCHTypeNumber(this.typeNumber);for(var i=0;i<18;i++){var mod=(!test&&((bits>>i)&1)==1);this.modules[Math.floor(i/3)][i%3+this.moduleCount-8-3]=mod;}
for(var i=0;i<18;i++){var mod=(!test&&((bits>>i)&1)==1);this.modules[i%3+this.moduleCount-8-3][Math.floor(i/3)]=mod;}},setupTypeInfo:function(test,maskPattern){var data=(this.errorCorrectLevel<<3)|maskPattern;var bits=QRUtil.getBCHTypeInfo(data);for(var i=0;i<15;i++){var mod=(!test&&((bits>>i)&1)==1);if(i<6){this.modules[i][8]=mod;}else if(i<8){this.modules[i+1][8]=mod;}else{this.modules[this.moduleCount-15+i][8]=mod;}}
for(var i=0;i<15;i++){var mod=(!test&&((bits>>i)&1)==1);if(i<8){this.modules[8][this.moduleCount-i-1]=mod;}else if(i<9){this.modules[8][15-i-1+1]=mod;}else{this.modules[8][15-i-1]=mod;}}
this.modules[this.moduleCount-8][8]=(!test);},mapData:function(data,maskPattern){var inc=-1;var row=this.moduleCount-1;var bitIndex=7;var byteIndex=0;for(var col=this.moduleCount-1;col>0;col-=2){if(col==6)col--;while(true){for(var c=0;c<2;c++){if(this.modules[row][col-c]==null){var dark=false;if(byteIndex<data.length){dark=(((data[byteIndex]>>>bitIndex)&1)==1);}
var mask=QRUtil.getMask(maskPattern,row,col-c);if(mask){dark=!dark;}
this.modules[row][col-c]=dark;bitIndex--;if(bitIndex==-1){byteIndex++;bitIndex=7;}}}
row+=inc;if(row<0||this.moduleCount<=row){row-=inc;inc=-inc;break;}}}}};QRCodeModel.PAD0=0xEC;QRCodeModel.PAD1=0x11;QRCodeModel.createData=function(typeNumber,errorCorrectLevel,dataList){var rsBlocks=QRRSBlock.getRSBlocks(typeNumber,errorCorrectLevel);var buffer=new QRBitBuffer();for(var i=0;i<dataList.length;i++){var data=dataList[i];buffer.put(data.mode,4);buffer.put(data.getLength(),QRUtil.getLengthInBits(data.mode,typeNumber));data.write(buffer);}
var totalDataCount=0;for(var i=0;i<rsBlocks.length;i++){totalDataCount+=rsBlocks[i].dataCount;}
if(buffer.getLengthInBits()>totalDataCount*8){throw new Error("code length overflow. ("
+buffer.getLengthInBits()
+">"
+totalDataCount*8
+")");}
if(buffer.getLengthInBits()+4<=totalDataCount*8){buffer.put(0,4);}
while(buffer.getLengthInBits()%8!=0){buffer.putBit(false);}
while(true){if(buffer.getLengthInBits()>=totalDataCount*8){break;}
buffer.put(QRCodeModel.PAD0,8);if(buffer.getLengthInBits()>=totalDataCount*8){break;}
buffer.put(QRCodeModel.PAD1,8);}
return QRCodeModel.createBytes(buffer,rsBlocks);};QRCodeModel.createBytes=function(buffer,rsBlocks){var offset=0;var maxDcCount=0;var maxEcCount=0;var dcdata=new Array(rsBlocks.length);var ecdata=new Array(rsBlocks.length);for(var r=0;r<rsBlocks.length;r++){var dcCount=rsBlocks[r].dataCount;var ecCount=rsBlocks[r].totalCount-dcCount;maxDcCount=Math.max(maxDcCount,dcCount);maxEcCount=Math.max(maxEcCount,ecCount);dcdata[r]=new Array(dcCount);for(var i=0;i<dcdata[r].length;i++){dcdata[r][i]=0xff&buffer.buffer[i+offset];}
offset+=dcCount;var rsPoly=QRUtil.getErrorCorrectPolynomial(ecCount);var rawPoly=new QRPolynomial(dcdata[r],rsPoly.getLength()-1);var modPoly=rawPoly.mod(rsPoly);ecdata[r]=new Array(rsPoly.getLength()-1);for(var i=0;i<ecdata[r].length;i++){var modIndex=i+modPoly.getLength()-ecdata[r].length;ecdata[r][i]=(modIndex>=0)?modPoly.get(modIndex):0;}}
var totalCodeCount=0;for(var i=0;i<rsBlocks.length;i++){totalCodeCount+=rsBlocks[i].totalCount;}
var data=new Array(totalCodeCount);var index=0;for(var i=0;i<maxDcCount;i++){for(var r=0;r<rsBlocks.length;r++){if(i<dcdata[r].length){data[index++]=dcdata[r][i];}}}
for(var i=0;i<maxEcCount;i++){for(var r=0;r<rsBlocks.length;r++){if(i<ecdata[r].length){data[index++]=ecdata[r][i];}}}
return data;};var QRMode={MODE_NUMBER:1<<0,MODE_ALPHA_NUM:1<<1,MODE_8BIT_BYTE:1<<2,MODE_KANJI:1<<3};var QRErrorCorrectLevel={L:1,M:0,Q:3,H:2};var QRMaskPattern={PATTERN000:0,PATTERN001:1,PATTERN010:2,PATTERN011:3,PATTERN100:4,PATTERN101:5,PATTERN110:6,PATTERN111:7};var QRUtil={PATTERN_POSITION_TABLE:[[],[6,18],[6,22],[6,26],[6,30],[6,34],[6,22,38],[6,24,42],[6,26,46],[6,28,50],[6,30,54],[6,32,58],[6,34,62],[6,26,46,66],[6,26,48,70],[6,26,50,74],[6,30,54,78],[6,30,56,82],[6,30,58,86],[6,34,62,90],[6,28,50,72,94],[6,26,50,74,98],[6,30,54,78,102],[6,28,54,80,106],[6,32,58,84,110],[6,30,58,86,114],[6,34,62,90,118],[6,26,50,74,98,122],[6,30,54,78,102,126],[6,26,52,78,104,130],[6,30,56,82,108,134],[6,34,60,86,112,138],[6,30,58,86,114,142],[6,34,62,90,118,146],[6,30,54,78,102,126,150],[6,24,50,76,102,128,154],[6,28,54,80,106,132,158],[6,32,58,84,110,136,162],[6,26,54,82,110,138,166],[6,30,58,86,114,142,170]],G15:(1<<10)|(1<<8)|(1<<5)|(1<<4)|(1<<2)|(1<<1)|(1<<0),G18:(1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<5)|(1<<2)|(1<<0),G15_MASK:(1<<14)|(1<<12)|(1<<10)|(1<<4)|(1<<1),getBCHTypeInfo:function(data){var d=data<<10;while(QRUtil.getBCHDigit(d)-QRUtil.getBCHDigit(QRUtil.G15)>=0){d^=(QRUtil.G15<<(QRUtil.getBCHDigit(d)-QRUtil.getBCHDigit(QRUtil.G15)));}
return((data<<10)|d)^QRUtil.G15_MASK;},getBCHTypeNumber:function(data){var d=data<<12;while(QRUtil.getBCHDigit(d)-QRUtil.getBCHDigit(QRUtil.G18)>=0){d^=(QRUtil.G18<<(QRUtil.getBCHDigit(d)-QRUtil.getBCHDigit(QRUtil.G18)));}
return(data<<12)|d;},getBCHDigit:function(data){var digit=0;while(data!=0){digit++;data>>>=1;}
return digit;},getPatternPosition:function(typeNumber){return QRUtil.PATTERN_POSITION_TABLE[typeNumber-1];},getMask:function(maskPattern,i,j){switch(maskPattern){case QRMaskPattern.PATTERN000:return(i+j)%2==0;case QRMaskPattern.PATTERN001:return i%2==0;case QRMaskPattern.PATTERN010:return j%3==0;case QRMaskPattern.PATTERN011:return(i+j)%3==0;case QRMaskPattern.PATTERN100:return(Math.floor(i/2)+Math.floor(j/3))%2==0;case QRMaskPattern.PATTERN101:return(i*j)%2+(i*j)%3==0;case QRMaskPattern.PATTERN110:return((i*j)%2+(i*j)%3)%2==0;case QRMaskPattern.PATTERN111:return((i*j)%3+(i+j)%2)%2==0;default:throw new Error("bad maskPattern:"+maskPattern);}},getErrorCorrectPolynomial:function(errorCorrectLength){var a=new QRPolynomial([1],0);for(var i=0;i<errorCorrectLength;i++){a=a.multiply(new QRPolynomial([1,QRMath.gexp(i)],0));}
return a;},getLengthInBits:function(mode,type){if(1<=type&&type<10){switch(mode){case QRMode.MODE_NUMBER:return 10;case QRMode.MODE_ALPHA_NUM:return 9;case QRMode.MODE_8BIT_BYTE:return 8;case QRMode.MODE_KANJI:return 8;default:throw new Error("mode:"+mode);}}else if(type<27){switch(mode){case QRMode.MODE_NUMBER:return 12;case QRMode.MODE_ALPHA_NUM:return 11;case QRMode.MODE_8BIT_BYTE:return 16;case QRMode.MODE_KANJI:return 10;default:throw new Error("mode:"+mode);}}else if(type<41){switch(mode){case QRMode.MODE_NUMBER:return 14;case QRMode.MODE_ALPHA_NUM:return 13;case QRMode.MODE_8BIT_BYTE:return 16;case QRMode.MODE_KANJI:return 12;default:throw new Error("mode:"+mode);}}else{throw new Error("type:"+type);}},getLostPoint:function(qrCode){var moduleCount=qrCode.getModuleCount();var lostPoint=0;for(var row=0;row<moduleCount;row++){for(var col=0;col<moduleCount;col++){var sameCount=0;var dark=qrCode.isDark(row,col);for(var r=-1;r<=1;r++){if(row+r<0||moduleCount<=row+r){continue;}
for(var c=-1;c<=1;c++){if(col+c<0||moduleCount<=col+c){continue;}
if(r==0&&c==0){continue;}
if(dark==qrCode.isDark(row+r,col+c)){sameCount++;}}}
if(sameCount>5){lostPoint+=(3+sameCount-5);}}}
for(var row=0;row<moduleCount-1;row++){for(var col=0;col<moduleCount-1;col++){var count=0;if(qrCode.isDark(row,col))count++;if(qrCode.isDark(row+1,col))count++;if(qrCode.isDark(row,col+1))count++;if(qrCode.isDark(row+1,col+1))count++;if(count==0||count==4){lostPoint+=3;}}}
for(var row=0;row<moduleCount;row++){for(var col=0;col<moduleCount-6;col++){if(qrCode.isDark(row,col)&&!qrCode.isDark(row,col+1)&&qrCode.isDark(row,col+2)&&qrCode.isDark(row,col+3)&&qrCode.isDark(row,col+4)&&!qrCode.isDark(row,col+5)&&qrCode.isDark(row,col+6)){lostPoint+=40;}}}
for(var col=0;col<moduleCount;col++){for(var row=0;row<moduleCount-6;row++){if(qrCode.isDark(row,col)&&!qrCode.isDark(row+1,col)&&qrCode.isDark(row+2,col)&&qrCode.isDark(row+3,col)&&qrCode.isDark(row+4,col)&&!qrCode.isDark(row+5,col)&&qrCode.isDark(row+6,col)){lostPoint+=40;}}}
var darkCount=0;for(var col=0;col<moduleCount;col++){for(var row=0;row<moduleCount;row++){if(qrCode.isDark(row,col)){darkCount++;}}}
var ratio=Math.abs(100*darkCount/moduleCount/moduleCount-50)/5;lostPoint+=ratio*10;return lostPoint;}};var QRMath={glog:function(n){if(n<1){throw new Error("glog("+n+")");}
return QRMath.LOG_TABLE[n];},gexp:function(n){while(n<0){n+=255;}
while(n>=256){n-=255;}
return QRMath.EXP_TABLE[n];},EXP_TABLE:new Array(256),LOG_TABLE:new Array(256)};for(var i=0;i<8;i++){QRMath.EXP_TABLE[i]=1<<i;}
for(var i=8;i<256;i++){QRMath.EXP_TABLE[i]=QRMath.EXP_TABLE[i-4]^QRMath.EXP_TABLE[i-5]^QRMath.EXP_TABLE[i-6]^QRMath.EXP_TABLE[i-8];}
for(var i=0;i<255;i++){QRMath.LOG_TABLE[QRMath.EXP_TABLE[i]]=i;}
function QRPolynomial(num,shift){if(num.length==undefined){throw new Error(num.length+"/"+shift);}
var offset=0;while(offset<num.length&&num[offset]==0){offset++;}
this.num=new Array(num.length-offset+shift);for(var i=0;i<num.length-offset;i++){this.num[i]=num[i+offset];}}
QRPolynomial.prototype={get:function(index){return this.num[index];},getLength:function(){return this.num.length;},multiply:function(e){var num=new Array(this.getLength()+e.getLength()-1);for(var i=0;i<this.getLength();i++){for(var j=0;j<e.getLength();j++){num[i+j]^=QRMath.gexp(QRMath.glog(this.get(i))+QRMath.glog(e.get(j)));}}
return new QRPolynomial(num,0);},mod:function(e){if(this.getLength()-e.getLength()<0){return this;}
var ratio=QRMath.glog(this.get(0))-QRMath.glog(e.get(0));var num=new Array(this.getLength());for(var i=0;i<this.getLength();i++){num[i]=this.get(i);}
for(var i=0;i<e.getLength();i++){num[i]^=QRMath.gexp(QRMath.glog(e.get(i))+ratio);}
return new QRPolynomial(num,0).mod(e);}};function QRRSBlock(totalCount,dataCount){this.totalCount=totalCount;this.dataCount=dataCount;}
QRRSBlock.RS_BLOCK_TABLE=[[1,26,19],[1,26,16],[1,26,13],[1,26,9],[1,44,34],[1,44,28],[1,44,22],[1,44,16],[1,70,55],[1,70,44],[2,35,17],[2,35,13],[1,100,80],[2,50,32],[2,50,24],[4,25,9],[1,134,108],[2,67,43],[2,33,15,2,34,16],[2,33,11,2,34,12],[2,86,68],[4,43,27],[4,43,19],[4,43,15],[2,98,78],[4,49,31],[2,32,14,4,33,15],[4,39,13,1,40,14],[2,121,97],[2,60,38,2,61,39],[4,40,18,2,41,19],[4,40,14,2,41,15],[2,146,116],[3,58,36,2,59,37],[4,36,16,4,37,17],[4,36,12,4,37,13],[2,86,68,2,87,69],[4,69,43,1,70,44],[6,43,19,2,44,20],[6,43,15,2,44,16],[4,101,81],[1,80,50,4,81,51],[4,50,22,4,51,23],[3,36,12,8,37,13],[2,116,92,2,117,93],[6,58,36,2,59,37],[4,46,20,6,47,21],[7,42,14,4,43,15],[4,133,107],[8,59,37,1,60,38],[8,44,20,4,45,21],[12,33,11,4,34,12],[3,145,115,1,146,116],[4,64,40,5,65,41],[11,36,16,5,37,17],[11,36,12,5,37,13],[5,109,87,1,110,88],[5,65,41,5,66,42],[5,54,24,7,55,25],[11,36,12],[5,122,98,1,123,99],[7,73,45,3,74,46],[15,43,19,2,44,20],[3,45,15,13,46,16],[1,135,107,5,136,108],[10,74,46,1,75,47],[1,50,22,15,51,23],[2,42,14,17,43,15],[5,150,120,1,151,121],[9,69,43,4,70,44],[17,50,22,1,51,23],[2,42,14,19,43,15],[3,141,113,4,142,114],[3,70,44,11,71,45],[17,47,21,4,48,22],[9,39,13,16,40,14],[3,135,107,5,136,108],[3,67,41,13,68,42],[15,54,24,5,55,25],[15,43,15,10,44,16],[4,144,116,4,145,117],[17,68,42],[17,50,22,6,51,23],[19,46,16,6,47,17],[2,139,111,7,140,112],[17,74,46],[7,54,24,16,55,25],[34,37,13],[4,151,121,5,152,122],[4,75,47,14,76,48],[11,54,24,14,55,25],[16,45,15,14,46,16],[6,147,117,4,148,118],[6,73,45,14,74,46],[11,54,24,16,55,25],[30,46,16,2,47,17],[8,132,106,4,133,107],[8,75,47,13,76,48],[7,54,24,22,55,25],[22,45,15,13,46,16],[10,142,114,2,143,115],[19,74,46,4,75,47],[28,50,22,6,51,23],[33,46,16,4,47,17],[8,152,122,4,153,123],[22,73,45,3,74,46],[8,53,23,26,54,24],[12,45,15,28,46,16],[3,147,117,10,148,118],[3,73,45,23,74,46],[4,54,24,31,55,25],[11,45,15,31,46,16],[7,146,116,7,147,117],[21,73,45,7,74,46],[1,53,23,37,54,24],[19,45,15,26,46,16],[5,145,115,10,146,116],[19,75,47,10,76,48],[15,54,24,25,55,25],[23,45,15,25,46,16],[13,145,115,3,146,116],[2,74,46,29,75,47],[42,54,24,1,55,25],[23,45,15,28,46,16],[17,145,115],[10,74,46,23,75,47],[10,54,24,35,55,25],[19,45,15,35,46,16],[17,145,115,1,146,116],[14,74,46,21,75,47],[29,54,24,19,55,25],[11,45,15,46,46,16],[13,145,115,6,146,116],[14,74,46,23,75,47],[44,54,24,7,55,25],[59,46,16,1,47,17],[12,151,121,7,152,122],[12,75,47,26,76,48],[39,54,24,14,55,25],[22,45,15,41,46,16],[6,151,121,14,152,122],[6,75,47,34,76,48],[46,54,24,10,55,25],[2,45,15,64,46,16],[17,152,122,4,153,123],[29,74,46,14,75,47],[49,54,24,10,55,25],[24,45,15,46,46,16],[4,152,122,18,153,123],[13,74,46,32,75,47],[48,54,24,14,55,25],[42,45,15,32,46,16],[20,147,117,4,148,118],[40,75,47,7,76,48],[43,54,24,22,55,25],[10,45,15,67,46,16],[19,148,118,6,149,119],[18,75,47,31,76,48],[34,54,24,34,55,25],[20,45,15,61,46,16]];QRRSBlock.getRSBlocks=function(typeNumber,errorCorrectLevel){var rsBlock=QRRSBlock.getRsBlockTable(typeNumber,errorCorrectLevel);if(rsBlock==undefined){throw new Error("bad rs block @ typeNumber:"+typeNumber+"/errorCorrectLevel:"+errorCorrectLevel);}
var length=rsBlock.length/3;var list=[];for(var i=0;i<length;i++){var count=rsBlock[i*3+0];var totalCount=rsBlock[i*3+1];var dataCount=rsBlock[i*3+2];for(var j=0;j<count;j++){list.push(new QRRSBlock(totalCount,dataCount));}}
return list;};QRRSBlock.getRsBlockTable=function(typeNumber,errorCorrectLevel){switch(errorCorrectLevel){case QRErrorCorrectLevel.L:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+0];case QRErrorCorrectLevel.M:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+1];case QRErrorCorrectLevel.Q:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+2];case QRErrorCorrectLevel.H:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+3];default:return undefined;}};function QRBitBuffer(){this.buffer=[];this.length=0;}
QRBitBuffer.prototype={get:function(index){var bufIndex=Math.floor(index/8);return((this.buffer[bufIndex]>>>(7-index%8))&1)==1;},put:function(num,length){for(var i=0;i<length;i++){this.putBit(((num>>>(length-i-1))&1)==1);}},getLengthInBits:function(){return this.length;},putBit:function(bit){var bufIndex=Math.floor(this.length/8);if(this.buffer.length<=bufIndex){this.buffer.push(0);}
if(bit){this.buffer[bufIndex]|=(0x80>>>(this.length%8));}
this.length++;}};var QRCodeLimitLength=[[17,14,11,7],[32,26,20,14],[53,42,32,24],[78,62,46,34],[106,84,60,44],[134,106,74,58],[154,122,86,64],[192,152,108,84],[230,180,130,98],[271,213,151,119],[321,251,177,137],[367,287,203,155],[425,331,241,177],[458,362,258,194],[520,412,292,220],[586,450,322,250],[644,504,364,280],[718,560,394,310],[792,624,442,338],[858,666,482,382],[929,711,509,403],[1003,779,565,439],[1091,857,611,461],[1171,911,661,511],[1273,997,715,535],[1367,1059,751,593],[1465,1125,805,625],[1528,1190,868,658],[1628,1264,908,698],[1732,1370,982,742],[1840,1452,1030,790],[1952,1538,1112,842],[2068,1628,1168,898],[2188,1722,1228,958],[2303,1809,1283,983],[2431,1911,1351,1051],[2563,1989,1423,1093],[2699,2099,1499,1139],[2809,2213,1579,1219],[2953,2331,1663,1273]];


/** Constructor */
function QRCode(options) {
  var instance = this;
  
  //Default options
  this.options = {
    padding: 4,
    width: 256, 
    height: 256,
    typeNumber: 4,
    color: "#000000",
    background: "#ffffff",
    ecl: "M"
  };
  
  //In case the options is string
  if (typeof options === 'string') {
    options = {
      content: options
    };
  }
  
  //Merge options
  if (options) {
    for (var i in options) {
      this.options[i] = options[i];
    }
  }
  
  if (typeof this.options.content !== 'string') {
    throw new Error("Expected 'content' as string!");
  }
  
  if (this.options.content.length === 0 /* || this.options.content.length > 7089 */) {
    throw new Error("Expected 'content' to be non-empty!");
  }
  
  if (!(this.options.padding >= 0)) {
    throw new Error("Expected 'padding' value to be non-negative!");
  }
  
  if (!(this.options.width > 0) || !(this.options.height > 0)) {
    throw new Error("Expected 'width' or 'height' value to be higher than zero!");
  }
  
  //Gets the error correction level
  function _getErrorCorrectLevel(ecl) {
    switch (ecl) {
        case "L":
          return QRErrorCorrectLevel.L;
          
        case "M":
          return QRErrorCorrectLevel.M;
          
        case "Q":
          return QRErrorCorrectLevel.Q;
          
        case "H":
          return QRErrorCorrectLevel.H;
          
        default:
          throw new Error("Unknwon error correction level: " + ecl);
      }
  }
  
  //Get type number
  function _getTypeNumber(content, ecl) {      
    var length = _getUTF8Length(content);
    
    var type = 1;
    var limit = 0;
    for (var i = 0, len = QRCodeLimitLength.length; i <= len; i++) {
      var table = QRCodeLimitLength[i];
      if (!table) {
        throw new Error("Content too long: expected " + limit + " but got " + length);
      }
      
      switch (ecl) {
        case "L":
          limit = table[0];
          break;
          
        case "M":
          limit = table[1];
          break;
          
        case "Q":
          limit = table[2];
          break;
          
        case "H":
          limit = table[3];
          break;
          
        default:
          throw new Error("Unknwon error correction level: " + ecl);
      }
      
      if (length <= limit) {
        break;
      }
      
      type++;
    }
    
    if (type > QRCodeLimitLength.length) {
      throw new Error("Content too long");
    }
    
    return type;
  }

  //Gets text length
  function _getUTF8Length(content) {
    var result = encodeURI(content).toString().replace(/\%[0-9a-fA-F]{2}/g, 'a');
    return result.length + (result.length != content ? 3 : 0);
  }
  
  //Generate QR Code matrix
  var content = this.options.content;
  var type = _getTypeNumber(content, this.options.ecl);
  var ecl = _getErrorCorrectLevel(this.options.ecl);
  this.qrcode = new QRCodeModel(type, ecl);
  this.qrcode.addData(content);
  this.qrcode.make();
}

/** Generates QR Code as SVG image */
QRCode.prototype.svg = function(opt) {
  var options = this.options || { };
  var modules = this.qrcode.modules;
  
  if (typeof opt == "undefined") {
    opt = { container: options.container || "svg" };
  }
  
  //Apply new lines and indents in SVG?
  var pretty = typeof options.pretty != "undefined" ? !!options.pretty : true;
  
  var indent = pretty ? '  ' : '';
  var EOL = pretty ? '\r\n' : '';
  var width = options.width;
  var height = options.height;
  var length = modules.length;
  var xsize = width / (length + 2 * options.padding);
  var ysize = height / (length + 2 * options.padding);
  
  //Join (union, merge) rectangles into one shape?
  var join = typeof options.join != "undefined" ? !!options.join : false;
  
  //Swap the X and Y modules, pull request #2
  var swap = typeof options.swap != "undefined" ? !!options.swap : false;
  
  //Apply <?xml...?> declaration in SVG?
  var xmlDeclaration = typeof options.xmlDeclaration != "undefined" ? !!options.xmlDeclaration : true;
  
  //Populate with predefined shape instead of "rect" elements, thanks to @kkocdko
  var predefined = typeof options.predefined != "undefined" ? !!options.predefined : false;
  var defs = predefined ? indent + '<defs><path id="qrmodule" d="M0 0 h' + ysize + ' v' + xsize + ' H0 z" style="fill:' + options.color + ';shape-rendering:crispEdges;" /></defs>' + EOL : '';
  
  //Background rectangle
  var bgrect = indent + '<rect x="0" y="0" width="' + width + '" height="' + height + '" style="fill:' + options.background + ';shape-rendering:crispEdges;"/>' + EOL;
  
  //Rectangles representing modules
  var modrect = '';
  var pathdata = '';

  for (var y = 0; y < length; y++) {
    for (var x = 0; x < length; x++) {
      var module = modules[x][y];
      if (module) {
        
        var px = (x * xsize + options.padding * xsize);
        var py = (y * ysize + options.padding * ysize);
        
        //Some users have had issues with the QR Code, thanks to @danioso for the solution
        if (swap) {
          var t = px;
          px = py;
          py = t;
        }
        
        if (join) {
          //Module as a part of svg path data, thanks to @danioso
          var w = xsize + px
          var h = ysize + py

          px = (Number.isInteger(px))? Number(px): px.toFixed(2);
          py = (Number.isInteger(py))? Number(py): py.toFixed(2);
          w = (Number.isInteger(w))? Number(w): w.toFixed(2);
          h = (Number.isInteger(h))? Number(h): h.toFixed(2);

          pathdata += ('M' + px + ',' + py + ' V' + h + ' H' + w + ' V' + py + ' H' + px + ' Z ');
        }
        else if (predefined) {
          //Module as a predefined shape, thanks to @kkocdko
          modrect += indent + '<use x="' + px.toString() + '" y="' + py.toString() + '" href="#qrmodule" />' + EOL;
        }
        else {
          //Module as rectangle element
          modrect += indent + '<rect x="' + px.toString() + '" y="' + py.toString() + '" width="' + xsize + '" height="' + ysize + '" style="fill:' + options.color + ';shape-rendering:crispEdges;"/>' + EOL;
        }
      }
    }
  }
  
  if (join) {
    modrect = indent + '<path x="0" y="0" style="fill:' + options.color + ';shape-rendering:crispEdges;" d="' + pathdata + '" />';
  }

  var svg = "";
  switch (opt.container) {
    //Wrapped in SVG document
    case "svg":
      if (xmlDeclaration) {
        svg += '<?xml version="1.0" standalone="yes"?>' + EOL;
      }
      svg += '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="' + width + '" height="' + height + '">' + EOL;
      svg += defs + bgrect + modrect;
      svg += '</svg>';
      break;
      
    //Viewbox for responsive use in a browser, thanks to @danioso
    case "svg-viewbox":
      if (xmlDeclaration) {
        svg += '<?xml version="1.0" standalone="yes"?>' + EOL;
      }
      svg += '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" viewBox="0 0 ' + width + ' ' + height + '">' + EOL;
      svg += defs + bgrect + modrect;
      svg += '</svg>';
      break;
      
    
    //Wrapped in group element    
    case "g":
      svg += '<g width="' + width + '" height="' + height + '">' + EOL;
      svg += defs + bgrect + modrect;
      svg += '</g>';
      break;
      
    //Without a container
    default:
      svg += (defs + bgrect + modrect).replace(/^\s+/, ""); //Clear indents on each line
      break;
  }
  
  return svg;
};

/** Writes QR Code image to a file */
QRCode.prototype.save = function(file, callback) {
  var data = this.svg();
  if (typeof callback != "function") {
    callback = function(error, result) { };
  }
  try {
    //Package 'fs' is available in node.js but not in a web browser
    var fs = require('fs');
    fs.writeFile(file, data, callback);
  }
  catch (e) {
    //Sorry, 'fs' is not available
    callback(e);
  }
};

if (typeof module != "undefined") {
  module.exports = QRCode;
}


/*\
|*|
|*|  Base64 / binary data / UTF-8 strings utilities
|*|
|*|  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Base64_encoding_and_decoding
|*|
\*/

/* Array of bytes to Base64 string decoding */

function b64ToUint6 (nChr) {

  return nChr > 64 && nChr < 91 ?
      nChr - 65
    : nChr > 96 && nChr < 123 ?
      nChr - 71
    : nChr > 47 && nChr < 58 ?
      nChr + 4
    : nChr === 43 ?
      62
    : nChr === 47 ?
      63
    :
      0;

}

function base64DecToArr (sBase64, nBlocksSize) {

  var
    sB64Enc = sBase64.replace(/[^A-Za-z0-9\+\/]/g, ""), nInLen = sB64Enc.length,
    nOutLen = nBlocksSize ? Math.ceil((nInLen * 3 + 1 >> 2) / nBlocksSize) * nBlocksSize : nInLen * 3 + 1 >> 2, taBytes = new Uint8Array(nOutLen);

  for (var nMod3, nMod4, nUint24 = 0, nOutIdx = 0, nInIdx = 0; nInIdx < nInLen; nInIdx++) {
    nMod4 = nInIdx & 3;
    nUint24 |= b64ToUint6(sB64Enc.charCodeAt(nInIdx)) << 6 * (3 - nMod4);
    if (nMod4 === 3 || nInLen - nInIdx === 1) {
      for (nMod3 = 0; nMod3 < 3 && nOutIdx < nOutLen; nMod3++, nOutIdx++) {
        taBytes[nOutIdx] = nUint24 >>> (16 >>> nMod3 & 24) & 255;
      }
      nUint24 = 0;

    }
  }

  return taBytes;
}

/* Base64 string to array encoding */

function uint6ToB64 (nUint6) {

  return nUint6 < 26 ?
      nUint6 + 65
    : nUint6 < 52 ?
      nUint6 + 71
    : nUint6 < 62 ?
      nUint6 - 4
    : nUint6 === 62 ?
      43
    : nUint6 === 63 ?
      47
    :
      65;

}

function base64EncArr (aBytes) {

  var nMod3 = 2, sB64Enc = "";

  for (var nLen = aBytes.length, nUint24 = 0, nIdx = 0; nIdx < nLen; nIdx++) {
    nMod3 = nIdx % 3;
    if (nIdx > 0 && (nIdx * 4 / 3) % 76 === 0) { sB64Enc += "\n"; }
    nUint24 |= aBytes[nIdx] << (16 >>> nMod3 & 24);
    if (nMod3 === 2 || aBytes.length - nIdx === 1) {
      sB64Enc += String.fromCodePoint(uint6ToB64(nUint24 >>> 18 & 63), uint6ToB64(nUint24 >>> 12 & 63), uint6ToB64(nUint24 >>> 6 & 63), uint6ToB64(nUint24 & 63));
      nUint24 = 0;
    }
  }

  return sB64Enc.substr(0, sB64Enc.length - 2 + nMod3) + (nMod3 === 2 ? '' : nMod3 === 1 ? '=' : '==');

}

// +------------------------------------------------------------------------+ //
// |                   COPY AND PASTED VENDOR CODE END                      | //
// +------------------------------------------------------------------------+ //

const env = UIHelperPrivate.buildEnvObject();

// Make the web UI behave more like a native UI 
if (! env.dev) {
    UIHelperPrivate.applyTweaks(env);
}

window.DISTRHO = {      // Namespace
    UI: UI,             // Base class for UIs, approximately mirrors C++ version.
    UIHelper: UIHelper, // Web browser oriented utility functions
    QRCode: QRCode,     // QR generator https://github.com/papnkukn/qrcode-svg
    Base64: {           // Base64 codec from MDN
        encode: base64EncArr,
        decode: base64DecToArr
    },
    env: env /* {       // Information about the environment (boolean values)
        plugin             True when running in plugin embedded web view
        network            True when communicating over the network (HTTP & WS)
        dev                True for non-plugin non-HTTP (ie. open file index.html)
        ...                Additional fields defined by web views
    } */
};

}); // new function()
