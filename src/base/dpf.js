R"DPF_JS(
/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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

{ // start private namespace

class DPF {

    // UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this._callNativeInternal('editParameter', index, started);
    }

    // UI::webViewScriptMessageReceived(ScriptValueDeque& args)
    callNative(...args) {
        window.webviewHost.postMessage([...args]);
    }

    // Private methods

    _callNativeInternal(method, ...args) {
        this.callNative('DPF', method, ...args);
    }

}

window.DPF = new DPF;

} // end private namespace

)DPF_JS"
