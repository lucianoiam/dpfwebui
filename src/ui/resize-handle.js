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

 class ResizeHandle {

    constructor(callback) {
        this.callback = callback;
        this.initialWidth = document.body.clientWidth;
        this.initialHeight = document.body.clientHeight;
        this.resizing = false;
        this.width = 0;
        this.height = 0;

        // FIXME - consider using a small SVG for displaying handle lines

        const handle = document.createElement('div');
        //handle.style.backgroundColor = 'rgba(0,0,0,0.5)';
        handle.style.position = 'fixed';
        handle.style.right = '0px';
        handle.style.bottom = '0px';
        handle.style.width = '24px';
        handle.style.height = '24px';
        handle.style.zIndex = '1000';

        document.body.appendChild(handle);

        handle.addEventListener('mousedown', (ev) => {
            this.resizing = true;
            this.width = document.body.clientWidth;
            this.height = document.body.clientHeight;
        });

        window.addEventListener('mouseup', (ev) => {
            this.resizing = false;
        });

        window.addEventListener('mousemove', (ev) => {
            if (!this.resizing) {
                return
            }

            const accel = 4; // REAPER works better when accel > 1
            
            const newWidth = Math.max(this.initialWidth, Math.min(window.screen.width,
                this.width + accel * ev.movementX));
            const newHeight = Math.max(this.initialHeight, Math.min(window.screen.height,
                this.height + accel * ev.movementY));

            if ((this.width != newWidth) || (this.height != newHeight)) {
                this.width = newWidth;
                this.height = newHeight;
                const k = window.devicePixelRatio;
                this.callback(k * this.width, k * this.height);
                //console.log(`${this.width}x${this.height}`);
            }
        });
    }

 }
