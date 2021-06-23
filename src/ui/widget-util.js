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

class WidgetUtil {

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
