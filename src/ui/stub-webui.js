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

class StubWebUI {

    static installIfNeeded() {
        if (typeof(DISTRHO_WebUI) == 'undefined') {
            console.log('DISTRHO_WebUI is not present, installing stub')
            window.DISTRHO_WebUI = StubWebUI;
        }
    }

    constructor() {
        // no-op
    }

    async getWidth() {
        return document.body.clientWidth;
    }

    async getHeight() {
        return document.body.clientHeight;
    }

    async isResizable() {
        return true;
    }

    setSize(width, height) {
        console.log(`setSize(${width}, ${height})`);
    }

    editParameter(index, started) {
        console.log(`editParameter(${index}, ${started})`);
    }

    setParameterValue(index, value) {
        console.log(`setParameterValue(${index}, ${value})`);
    }

    setState(key, value) {
        console.log(`setState(${key}, ${value})`);
    }

    flushInitMessageQueue() {
        console.log('flushInitMessageQueue()');
    }

    postMessage(...args) {
        console.log(`postMessage(${args})`);
    }

}

StubWebUI.installIfNeeded();
