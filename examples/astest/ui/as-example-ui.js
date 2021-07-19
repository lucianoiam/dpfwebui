/*
 * Apices - Audio Plugins In C++ & ES6
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

// See src/js/ui.js for the DISTRHO_UI class implementation, it shows which C++
// methods are mapped. The class is automatically injected into the global
// namespace and guaranteed to be available after the document load event fires
// and before any referenced script starts running.

class AsExampleUI extends DISTRHO_UI {

    constructor() {
        super(); // mandatory

        // Helper function
        const elem = (id) => document.getElementById(id);

        // Connect slider to plugin
        elem('gain-slider').addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });

        // Show user agent
        elem('user-agent').innerText = navigator.userAgent;

        // How to properly handle keyboard focus: when the web widget is not
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

        // Process any UI message generated while the web view was still loading
        // It is mandatory to call this method at some point, e.g. after UI gets
        // ready, otherwise messages get indefinitely accumulated on C++ side.
        this.flushInitMessageQueue();

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
