/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

// See src/ui/dpf.js for the UI class implementation

class WebGainExampleUI extends DISTRHO.UI {

    constructor() {
        super();

        // Helper function
        const elem = (id) => document.getElementById(id);

        // Connect slider to plugin
        elem('gain-slider').addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });

        // Show user agent
        elem('user-agent').innerText = navigator.userAgent;

        // How to properly handle keyboard focus: when the web view is not
        // accepting keyboard input (default setting) keystrokes are routed to
        // the host. This allows for example to play with the virtual Live
        // keyboard. When the widget needs keyboard this.setKeyboardFocus(true)
        // should be called and this.setKeyboardFocus(false) when done. 
        elem('kbd-demo').addEventListener('focus', ev => this.setKeyboardFocus(true));
        elem('kbd-demo').addEventListener('blur', ev => this.setKeyboardFocus(false));

        elem('smiley').addEventListener('click', ev => {
            ev.target.style.display = 'none';
            elem('kbd-demo').style.display = 'inline';
            elem('kbd-demo').focus();
        });

        // See WebGainExampleUI constructor in WebGainExampleUI.cpp
        testInjectedFunction();

        // Showtime
        document.body.style.visibility = 'visible';
    }

    parameterChanged(index, value) {
        switch (index) {
            case 0:
                document.getElementById('gain-slider').value = value;
                break;
            default:
                break;
        }
    }

}

//
// Optional: workaround for input[type=range] sliders on Linux touchscreen
//
// Not sure this is a bug or feature of WebKitGTK. These elements do not
// react to touches on Linux but do on other platforms. It does not seem to
// be a dpfwebui bug as touch works as expected for every other element.
//
if (DISTRHO.env.noRangeInputTouch) {
    document.querySelectorAll('input[type=range]').forEach((el) => {
        el.addEventListener('touchmove', (ev) => {
            const minVal = parseFloat(ev.target.min);
            const maxVal = parseFloat(ev.target.max);
            const width = ev.target.offsetWidth;
            const x = ev.touches[0].clientX;                
            const minX = ev.target.getBoundingClientRect().x;
            const maxX = minX + width;
            if ((x < minX) || (x > maxX)) return;
            const normVal = (x - minX) / width;
            const val = minVal + normVal * (maxVal - minVal);
            ev.target.value = val;
            ev.target.dispatchEvent(new CustomEvent('input'));
        });
    });
}
