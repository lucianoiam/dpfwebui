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

    constructor(callback, options) {
        this.callback = callback; 

        options = options || {};

        // Default minimum size is the current content size
        this.minWidth = options.minWidth || document.body.clientWidth;
        this.minHeight = options.minHeight || document.body.clientHeight;

        if (options.maxScale) {
            // Set the maximum size to maxScale times the minimum size 
            this.maxWidth = options.maxScale * this.minWidth;
            this.maxHeight = options.maxScale * this.minHeight;
        } else {
            // Default maximum size is the device screen size
            this.maxWidth = options.maxWidth || window.screen.width;
            this.maxHeight = options.maxHeight || window.screen.height;
        }

        // Keep aspect ratio while resizing, default to yes
        this.keepAspectRatio = options.keepAspectRatio === false ? false : true;
        this.aspectRatio = this.minWidth / this.minHeight;

        this.width = 0;
        this.height = 0;
        this.resizing = false;

        this.handle = this._createHandle(options.theme || 'dots');

        this._addEventListeners();
    }

    get element() {
        return this.handle;
    }

    _createHandle(theme) {
        const handle = document.createElement('div');
        
        switch (theme) {
            case 'dots':
                handle.innerHTML = ResizeHandle.themeSvgData.DOTS;
                break;
            case 'lines':
                handle.innerHTML = ResizeHandle.themeSvgData.LINES;
                break;
            default:
                break;
        }

        handle.style.position = 'fixed';
        handle.style.zIndex = '1000';
        handle.style.right = '0px';
        handle.style.bottom = '0px';
        handle.style.width = '24px';
        handle.style.height = '24px';

        return handle;
    }

    _addEventListeners() {
        const evOptions = {passive: false};

        ['touchstart', 'mousedown'].forEach((evName) => {
            this.handle.addEventListener(evName, (ev) => {
                this._onDragStart(ev);
                if (ev.cancelable) {
                    ev.preventDefault(); // first handled event wins
                }
            }, evOptions);
        });

        ['touchmove', 'mousemove'].forEach((evName) => {
            window.addEventListener(evName, (ev) => {
                // FIXME: On Windows, touchmove events stop triggering after calling callback,
                //        which in turn calls DistrhoUI::setSize(). Mouse resizing works OK.
                this._onDragContinue(ev);
                if ((ev.target == this.handle) && ev.cancelable) {
                    ev.preventDefault();
                }
            }, evOptions);
        });

        ['touchend', 'mouseup'].forEach((evName) => {
            window.addEventListener(evName, (ev) => {
                this._onDragEnd(ev);
                if ((ev.target == this.handle) && ev.cancelable) {
                    ev.preventDefault();
                }
            }, evOptions);
        });
    }

    _onDragStart(ev) {
        this.resizing = true;

        this.width = document.body.clientWidth;
        this.height = document.body.clientHeight;

        this.x = ev.clientX /* mouse */ || ev.touches[0].clientX;
        this.y = ev.clientY /* mouse */ || ev.touches[0].clientY;
    }

    _onDragContinue(ev) {
        if (!this.resizing) {
            return
        }

        const clientX = ev.clientX || ev.touches[0].clientX;
        const dx = clientX - this.x;
        this.x = clientX;
        let newWidth = Math.max(this.minWidth, Math.min(this.maxWidth, this.width + dx));

        const clientY = ev.clientY || ev.touches[0].clientY;
        const dy = clientY - this.y;
        this.y = clientY;
        let newHeight = Math.max(this.minHeight, Math.min(this.maxHeight, this.height + dy));

        if (this.keepAspectRatio) {
            if (dx > dy) {
                newHeight = newWidth / this.aspectRatio;
            } else {
                newWidth = newHeight * this.aspectRatio;
            }
        }

        if ((this.width != newWidth) || (this.height != newHeight)) {
            this.width = newWidth;
            this.height = newHeight;
            const k = window.devicePixelRatio;
            this.callback(k * this.width, k * this.height);
        }
    }

    _onDragEnd(ev) {
        this.resizing = false;
    }

}

ResizeHandle.themeSvgData = Object.freeze({
    DOTS: `
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
            <path opacity="0.25" d="M80.5,75.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                C78.262,70.5,80.5,72.74,80.5,75.499z"/>
            <path opacity="0.25" d="M50.5,75.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                C48.262,70.5,50.5,72.74,50.5,75.499z"/>
            <path opacity="0.25" d="M80.5,45.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                C78.262,40.5,80.5,42.74,80.5,45.499z"/>
        </svg>`
    ,
    LINES: `
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
            <line stroke="#000000" opacity="0.5" x1="0" y1="100" x2="100" y2="0"/>
            <line stroke="#000000" opacity="0.5" x1="100" y1="25" x2="25" y2="100"/>
            <line stroke="#000000" opacity="0.5" x1="50" y1="100" x2="100" y2="50"/>
            <line stroke="#000000" opacity="0.5" x1="75" y1="100" x2="100" y2="75"/>
        </svg>`
});
