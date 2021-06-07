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

{ // start private namespace

class DPF {

    constructor() {
        // TODO

        this.addHostMessageListener((message) => {
            console.log(message);
        });
    }

    // UI::editParameter(uint32_t index, bool started)
    editParameter(index, started) {
        this._builtinCall('editParameter', index, started);
    }

    /**
      Convenience methods
     */

    // WebViewEventHandler::webViewScriptMessageReceived(const ScriptValueVector& args)
    postScriptMessage(...args) {
        window.webviewHost.postMessage([...args]);
    }

    // BaseWebView::postHostMessage(const ScriptValueVector& args)
    addHostMessageListener(listener) {
        window.webviewHost.addEventListener('message', (ev) => listener(ev.detail));
    }

    /**
      Private methods
     */

    _builtinCall(method, ...args) {
        this.postScriptMessage('DPF', method, ...args);
    }

}

window.DPF = new DPF;

} // end private namespace

)DPF_JS"
