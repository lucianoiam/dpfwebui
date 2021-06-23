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

            const accel = 1; // REAPER works better when accel > 1

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
