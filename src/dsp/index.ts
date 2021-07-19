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

import PluginImpl from './plugin';

const instance = new PluginImpl;

export function getLabel(): ArrayBuffer {
    return String.UTF8.encode(instance.getLabel(), true);
}

export function getMaker(): ArrayBuffer {
    return String.UTF8.encode(instance.getMaker(), true);
}

export function getLicense(): ArrayBuffer {
    return String.UTF8.encode(instance.getLicense(), true);
}
