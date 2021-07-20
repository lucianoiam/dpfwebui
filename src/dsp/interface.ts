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

export default interface DISTRHO_Plugin {

    // const char* Plugin::getLabel()
    getLabel(): string

    // const char* Plugin::getMaker()
    getMaker(): string

    // const char* Plugin::getLicense()
    getLicense(): string

    // void Plugin::run(const float** inputs, float** outputs, uint32_t frames)
    run(inputs: Float32Array[], outputs: Float32Array[]): void

}