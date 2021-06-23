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

// See src/base/webui.js for the DISTRHO_WebUI class implementation, it shows
// which C++ methods are mapped. The class is automatically injected into the
// global namespace and guaranteed to be available after the document load event
// fires and before any referenced script starts running.

class WebGainExampleUI extends DISTRHO_WebUI {

    constructor() {
        super(); // do not forget
        
        // This is just a shortcut to avoid repeating document.getElementById()
        this.dom = Object.freeze({
            userAgent:  document.getElementById('user-agent'),
            gainSlider: document.getElementById('gain-slider')
        });

        // Fix input[type=range] sliders not reacting to touch events on Linux
        Platform.fixLinuxInputTypeRangeTouch();

        // Mostly connect controls to the plugin
        this._setupView();
        
        // Allow the user to resize the plugin window
        this._addResizeHandle();

        // Process any UI message generated while the web view was still loading
        // It is mandatory to call this method at some point, e.g. after UI gets
        // ready, otherwise messages will accumulate indefinitely on C++ side.
        this.flushInitMessageQueue();

        // Showtime
        document.body.style.visibility = 'visible';
    }

    parameterChanged(index, value) {
        switch (index) {
            case 0:
                this.dom.gainSlider.value = value;
                break;
            default:
                break;
        }
    }

    _setupView() {
        this.dom.userAgent.innerText = navigator.userAgent;

        this.dom.gainSlider.addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });
    }

    async _addResizeHandle() {
        const k = window.devicePixelRatio;
        const options = {
            minWidth:  /* UI::getWidth()  */ await this.getWidth()  / k,
            minHeight: /* UI::getHeight() */ await this.getHeight() / k,
            maxScale: 2,
            keepAspectRatio: true
        };
        const handle = new ResizeHandle((w, h) => this.setSize(w, h), options);
        document.body.appendChild(handle.element);
    }

}
