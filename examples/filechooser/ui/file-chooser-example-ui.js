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

class FileChooserExampleUI extends DISTRHO.UI {

    constructor() {
        super();

        document.getElementById('choose').addEventListener('click', (ev) => {
            this.requestStateFile('file');
        });
    }

    stateChanged(key, value) {
        if ((key == 'file') && value) {
            document.getElementById('filename').innerText = value;
        }
    }

}
