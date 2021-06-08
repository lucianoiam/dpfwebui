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

class WebExampleUI extends WebUI {

    constructor() {
        super();

        this.colors     = Object.freeze(['#f00', '#0f0', '#00f']);
        this.colorIndex = 0;

        const main = document.createElement('div');
        main.id = 'main';
        
        main.innerHTML = `
            <h1>Made with DPF</h1>
            <img src="music.gif">
            <pre id="ua">${navigator.userAgent}</pre>
        `;

        main.addEventListener('mousedown', (ev) => {
            const i = this.colorIndex++ % this.colors.length;
            ev.currentTarget.style.borderColor = this.colors[i];
        });

        document.body.appendChild(main);
        
        
        // TEST CALL
        this.editParameter(123,true);
    }

    parameterChanged(index, value) {
        console.log(`WEB received parameterChanged(${index},${value})`);
    }

}
