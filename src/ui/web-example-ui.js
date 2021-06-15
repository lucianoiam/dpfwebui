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

class WebExampleUI extends DISTRHO_WebUI {

    constructor() {
        super(); // do not forget to call

        this.colors     = Object.freeze(['#f00', '#0f0', '#00f']);
        this.colorIndex = 0;

        const main = document.createElement('div');
        main.id = 'main';
        
        main.innerHTML = `
            <h1>Made with DPF</h1>
            <img style="width:32px; height:25px;" src="music.gif">
            <pre id="ua">${navigator.userAgent}</pre>
        `;

        main.addEventListener('mousedown', (ev) => {
            const i = this.colorIndex++ % this.colors.length;
            ev.currentTarget.style.borderColor = this.colors[i];
        });

        const body = document.body;

        body.appendChild(main);
        body.style.visibility = 'visible';

        // Not needed for this demo but any plugin implementing parameters
        // should call this to trigger any parameter change callbacks that may
        // have accumulated while the web view was still loading. 
        // TODO: probably the same goes for state callacks -- need to try
        this.flushInitMessageQueue();
    }

}
