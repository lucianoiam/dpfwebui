/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

const env = DISTRHO.env, helper = DISTRHO.UIHelper;

// ZamCompX2Plugin.hpp@33
const PARAMETERS = [
    'attack', 'release', 'knee', 'ratio', 'threshold', 'makeup', 'slew',
    'stereo', 'sidechain', 'gain', 'output-level'
];

class ZCompExampleUI extends DISTRHO.UI {

    constructor() {
        super();

        // Automatically display a modal view when connection is lost
        helper.enableOfflineModal(this);

        // Do not navigate when clicking credits and open system browser instead
        helper.bindSystemBrowser(this, document.querySelector('#credits > a'));

        // Setup view to suit environment
        if (env.plugin) {
            this._setupForPluginEmbeddedWebView();
        } else if (env.network) {
            this._setupForNetworkClients();
        } else if (env.dev) {
            this._setupForDevelopment();
        }

        // Make the knobs control plugin parameters
        this._addKnobListeners();
    }

    parameterChanged(index, value) {
        // Update knob value
        const knob = this._getKnob(index);
        if (knob) {
            knob.value = value;
        }

        // Do not show UI until full initial parameter state has been received
        this._parameterChangeCount = this._parameterChangeCount || 0;
        if (++this._parameterChangeCount == PARAMETERS.length) {
            document.body.style.visibility = 'visible';
        }
    }

    _setupForPluginEmbeddedWebView() {
        // Add a button for displaying a QR code with the plugin URL
        const main = document.getElementById('main');
        main.appendChild(helper.getNetworkDetailsModalButton(this, {
            fill: '#000',
            id: 'qr-button',
            modal: {
                id: 'qr-modal'
            }
        }));
    }

    _setupForNetworkClients() {
        // Center the user interface within the web browser viewport
        document.getElementById('overscan').style.background = 'rgba(0,0,0,0.5)';
        const main = document.getElementById('main');
        main.style.borderRadius = '10px';
        main.style.width = '640px';
        main.style.height = '128px';
        document.body.style.minWidth = main.style.width;
        document.body.style.minHeight = main.style.height;
    }

    _setupForDevelopment() {
        // Directly Open the Source mode ;)
        const main = document.getElementById('main');
        main.innerHTML = 'This program cannot be run in DOS mode';
        document.body.style.visibility = 'visible';
    }

    _addKnobListeners() {
        // Some parameters in ZamCompX2 are missing from this example
        for (let i = 0; i < PARAMETERS.length; i++) {
            const knob = this._getKnob(i);
            if (knob) {
                knob.addEventListener('input', (ev) => {
                    this.setParameterValue(i, ev.target.value);
                });
            }
        }
    }

    _getKnob(parameterIndex) {
        return document.getElementById(`knob-${PARAMETERS[parameterIndex]}`);
    }

}
