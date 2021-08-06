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

class JitDrumExampleUI extends DISTRHO_UI {

    constructor() {
        super();

        const elem = (id) => document.getElementById(id);

        elem('note-1').addEventListener('click', (ev) => {
            this.sendNote(1, 48, 127);
        });

        elem('note-2').addEventListener('click', (ev) => {
            this.sendNote(1, 60, 127);
        });

        elem('note-3').addEventListener('click', (ev) => {
            this.sendNote(1, 72, 127);
        });

        this.flushInitMessageQueue();
    }

}
