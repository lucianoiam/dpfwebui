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

class Platform {

    static fixLinuxInputTypeRangeTouch() {
        // Is this a bug or by design of WebKitGTK ? input[type=range] sliders
        // are not reacting to touches on that web view. It does not seem to be
        // a dpf-webui bug as touch works as expected for every other element.

        if (!/linux/i.test(window.navigator.platform)) {
            return;
        }

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

}
