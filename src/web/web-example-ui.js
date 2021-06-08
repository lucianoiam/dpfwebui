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
    }

    parameterChanged(index, value) {
        // empty
    }

}
