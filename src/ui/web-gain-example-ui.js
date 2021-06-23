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

// See src/base/webui.js for the DISTRHO_WebUI class implementation, it shows
// which C++ methods are mapped. The class is automatically injected into the
// global namespace and guaranteed to be available after the document load event
// fires and before any referenced script starts running.

class WebGainExampleUI extends DISTRHO_WebUI {

    constructor() {
        super(); // do not forget

        this.dom = {
            userAgent:  document.getElementById('user-agent'),
            gainSlider: document.getElementById('gain-slider')
        };

        this.dom.userAgent.innerText = navigator.userAgent;

        this.dom.gainSlider.addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });

        // Process any UI message generated while the web view was still loading
        // It is mandatory to call this method at some point, e.g. after UI gets
        // ready, otherwise messages will accumulate indefinitely on C++ side.
        this.flushInitMessageQueue();

        // Fix for input[type=range] sliders not reacting to touch events on Linux
        Platform.fixLinuxInputTypeRangeTouch();

        document.body.style.visibility = 'visible';

        this.isResizable().then((result) => {
            const handle = new ResizeHandle((w, h) => this.setSize(w, h) /* FIXME: pass min/max w/h */);
            document.body.appendChild(handle);
        });
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

}
