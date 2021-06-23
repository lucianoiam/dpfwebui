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

// For some yet unknown reason REAPER makes the resize handle react slowly.
// Behavior not reproducible on standalone application or plugin running on Live.
// Allow pressing shift when clicking on the handle to resize in larger steps.
const ResizeHandle_SHIFT_ACCELERATION = 4;

const ResizeHandle_SVG = `
<svg version="1.1" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
    <line stroke="#000000" stroke-opacity="0.5" x1="0" y1="100" x2="100" y2="0"/>
    <line stroke="#000000" stroke-opacity="0.5" x1="100" y1="25" x2="25" y2="100"/>
    <line stroke="#000000" stroke-opacity="0.5" x1="50" y1="100" x2="100" y2="50"/>
    <line stroke="#000000" stroke-opacity="0.5" x1="75" y1="100" x2="100" y2="75"/>
</svg>
`;

class ResizeHandle {

    constructor(callback) {
        this.callback = callback; 
        this.initialWidth = document.body.clientWidth;
        this.initialHeight = document.body.clientHeight;
        this.accel = 0;
        this.width = 0;
        this.height = 0;

        // FIXME - consider using a small SVG for displaying handle lines

        const handle = document.createElement('div');
        handle.innerHTML = ResizeHandle_SVG;
        handle.style.position = 'fixed';
        handle.style.right = '0px';
        handle.style.bottom = '0px';
        handle.style.width = '24px';
        handle.style.height = '24px';
        handle.style.zIndex = '1000';

        document.body.appendChild(handle);

        handle.addEventListener('mousedown', (ev) => {
            this.accel = ev.shiftKey ? ResizeHandle_SHIFT_ACCELERATION : 1;
            this.width = document.body.clientWidth;
            this.height = document.body.clientHeight;
        });

        window.addEventListener('mouseup', (ev) => {
            this.accel = 0;
        });

        window.addEventListener('mousemove', (ev) => {
            if (this.accel == 0) {
                return
            }

            const newWidth = Math.max(this.initialWidth, Math.min(window.screen.width,
            this.width + this.accel * ev.movementX));
            const newHeight = Math.max(this.initialHeight, Math.min(window.screen.height,
            this.height + this.accel * ev.movementY));

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
