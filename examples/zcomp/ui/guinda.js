/*
 * Guinda - Audio widgets for web views
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

(() => {

 /**
  *  Base class for all widgets
  */

class Widget extends HTMLElement {

    /**
     *  Public
     */

    static defineCustomElement() {
        this._initialize();
        window.customElements.define(`g-${this._unqualifiedNodeName}`, this);
    }

    constructor(props) {
        super();

        // Set properties and start observing changes. Properties passed to the
        // constructor will be overwritten by matching HTML attributes before
        // connectedCallback() is called.

        let propLock = false;

        this._props = new Proxy(props || {}, {
            set: (obj, prop, value) => {
                obj[prop] = value;

                if (! propLock) {
                    propLock = true;
                    this._propertyUpdated(prop, value);
                    propLock = false;
                } else {
                    // Avoid recursion, this._propertyUpdated()
                    // can in turn update properties.
                }

                return true;
            }
        });

        // Fill in any missing property values using defaults

        for (const desc of this.constructor._attributeDescriptors) {
            if (!(desc.key in this.props) && (typeof(desc.default) !== 'undefined')) {
                this.props[desc.key] = desc.default;
            }
        }

        // Style property changes can be observed with MutationObserver but it
        // only works for built-in style properties and not custom as of 2022.
    }

    get props() {
        return this._props;
    }

    set props(props) {
        Object.assign(this._props, props); // merge
    }

    // Custom element lifecycle callbacks

    static get observedAttributes() {
        const This = this.prototype.constructor;
        return This._attributeDescriptors.map(d => d.key.toLowerCase());
    }

    attributeChangedCallback(name, oldValue, newValue) {
        const This = this.constructor;
        const desc = This._attributeDescriptors.find(d => name == d.key.toLowerCase());

        if (desc) {
            const val = desc.parser(newValue, null);

            if (val !== null) {
                this.props[desc.key] = val;
            }
        }
    }

    connectedCallback() {
        this._root = this.attachShadow({mode: 'open'});
    }

    /**
     *  Internal
     */

    static get _unqualifiedNodeName() {
        throw new TypeError(`_unqualifiedNodeName() not implemented for ${this.name}`);
    }

    static get _attributeDescriptors() {
        return [];
    }

    static _initialize() {
        // Default empty implementation
    }

    _attr(name, def) {
        const attr = this.attributes.getNamedItem(name);
        return attr ? attr.value : def;
    }

    _parsedAttr(name) {
        const attrDesc = this.constructor._attributeDescriptors.find(d => d.key == name);
        return (typeof(attrDesc) !== 'undefined') ?
            attrDesc.parser(this._attr(name), attrDesc.default) : null;
    }

    _style(name, def) {
        const prop = getComputedStyle(this).getPropertyValue(name).trim();
        return prop.length > 0 ? prop : def;
    }

    _propertyUpdated(key, value) {
        // Default empty implementation
    }

    _redraw() {
        // Default empty implementation
    }

}


/**
 *  Base class for widgets that store a value
 */

class StatefulWidget extends Widget {

    /**
     *  Public
     */

    constructor(props) {
        super(props);

        // Needed for libraries which overwrite this.value, e.g. LemonadeJS
        // https://jsfiddle.net/3ad5q6cz/10/
        //this._value = this.value;
        //delete this.value;

        this._value = null;
    }
    
    connectedCallback() {
        super.connectedCallback();

        if (typeof(this._value) !== 'number') { // may have value before attaching
            this.value = this._parsedAttr('value'); // initial value
        }
    }

    get value() {
        return this._value;
    }

    set value(newValue) {
        this._value = newValue;
        this._valueUpdated();
    }

    /**
     *  Internal
     */

    _valueUpdated() {
        if (! this._root) {
            return;
        }

        this._redraw();
    }

}


/**
 *  Base class for widgets that accept an input value
 */

class InputWidget extends StatefulWidget {

    /**
     *  Public
     */

    constructor(props) {
        super(props);
        ControlTrait.apply(this, [props]);
    }

    /**
     *  Internal
     */

    _setValueAndDispatchInputEventIfNeeded(newValue) {
        if (this._value == newValue) {
            return;
        }

        this.value = newValue;
        this._dispatchInputEvent(newValue);
    }

    _dispatchInputEvent(val) {
        const ev = new Event('input');
        ev.value = val;
        this.dispatchEvent(ev);
    }

}


/**
 *  Base class for widgets that handle a value within a range
 */

class RangeInputWidget extends InputWidget {

    /**
     *  Public
     */
        
    constructor(props) {
        super(props);
        this._scaledValue = null;
    }

    get value() {
        return this._denormalize(super.value);
    }

    set value(newValue) {
        this._scaledValue = newValue;
        super.value = this._normalize(this._clamp(newValue));
    }

    /**
     *  Internal
     */

    static get _attributeDescriptors() {
        return super._attributeDescriptors.concat([
            { key: 'value', parser: ValueParser.float, default: 0 },
            { key: 'min'  , parser: ValueParser.float, default: 0 },
            { key: 'max'  , parser: ValueParser.float, default: 1 },
            { key: 'scale', parser: ValueParser.scale, default: ValueScale.linear }
        ]);
    }

    _propertyUpdated(key, value) {
        super._propertyUpdated(key, value);
        this.value = this._scaledValue;
    }

    _setNormalizedValueAndDispatchInputEventIfNeeded(newValue) {
        if (this._value == newValue) {
            return;
        }

        // Do not use this.value since newValue is already normalized [0-1.0]
        this._value = newValue;
        this._valueUpdated();

        this._dispatchInputEvent(this.value);
    }

    _clamp(value) {
        if (typeof value !== 'number') return value;
        return Math.max(this.props.min, Math.min(this.props.max, value));
    }

    _normalize(value) {
        if (typeof value !== 'number') return value;
        return this._scale.normalize(value, this.props.min, this.props.max);
    }

    _denormalize(value) {
        if (typeof value !== 'number') return value;
        return this._scale.denormalize(value, this.props.min, this.props.max);
    }

    get _scale() {
        return this.props.scale || ValueScale.linear;
    }

}


/**
 *  Traits
 */

class ControlEvent extends UIEvent {}

// Merges touch and mouse input events into a single basic set of custom events

function ControlTrait(props) {
    props = props || {}; // currently unused

    this._controlStarted = false;
    this._controlTimeout = null;

    // Synthesize a getter to keep this._controlStarted effectively private

    Object.defineProperty(this, 'isControlStarted', { get: () => this._controlStarted });

    // Handle touch events preventing subsequent simulated mouse events

    this.addEventListener('touchstart', (ev) => {
        dispatchControlStart(ev, ev.touches[0].clientX, ev.touches[0].clientY);

        if (ev.cancelable) {
            ev.preventDefault();
        }
    });

    this.addEventListener('touchmove', (ev) => {
        dispatchControlContinue(ev, ev.touches[0].clientX, ev.touches[0].clientY);

        if (ev.cancelable) {
            ev.preventDefault();
        }
    });
    
    this.addEventListener('touchend', (ev) => {
        dispatchControlEnd(ev);

        if (ev.cancelable) {
            ev.preventDefault();
        }
    });

    // Simulate touch behavior for mouse, for example react to move events outside element

    this.addEventListener('mousedown', (ev) => {
        if (ev.button == 0) {
            window.addEventListener('mousemove', mouseMoveListener);
            window.addEventListener('mouseup', mouseUpListener);

            dispatchControlStart(ev, ev.clientX, ev.clientY);
        }
    });

    // Special treatment for wheel: custom start, continue and end events

    this.addEventListener('wheel', (ev) => {
        if (!this._controlStarted) {
            dispatchControlStart(ev, ev.clientX, ev.clientY);
        }

        const k = ev.shiftKey ? 2 : 1;
        const inv = ev.webkitDirectionInvertedFromDevice ? -1 : 1;
        const clientX = this._prevClientX + k * inv * Math.sign(ev.deltaX);
        const clientY = this._prevClientY + k * inv * Math.sign(ev.deltaY);
        
        dispatchControlContinue(ev, clientX, clientY);

        if (this._controlTimeout) {
            clearTimeout(this._controlTimeout);
        }

        this._controlTimeout = setTimeout(() => {
            this._controlTimeout = null;
            dispatchControlEnd(ev);
        }, 100);

        ev.preventDefault();
    });

    const mouseMoveListener = (ev) => {
        dispatchControlContinue(ev, ev.clientX, ev.clientY);
    };

    const mouseUpListener = (ev) => {
        window.removeEventListener('mouseup', mouseUpListener);
        window.removeEventListener('mousemove', mouseMoveListener);

        dispatchControlEnd(ev);
    };

    const dispatchControlStart = (originalEvent, clientX, clientY) => {
        this._controlStarted = true;

        this._prevClientX = clientX;
        this._prevClientY = clientY;

        const ev = createControlEvent('controlstart', originalEvent, clientX, clientY);

        this.dispatchEvent(ev);
    };

    const dispatchControlContinue = (originalEvent, clientX, clientY) => {
        const ev = createControlEvent('controlcontinue', originalEvent, clientX, clientY);
        
        ev.deltaX = clientX - this._prevClientX;
        ev.deltaY = clientY - this._prevClientY;

        this._prevClientX = clientX;
        this._prevClientY = clientY;

        this.dispatchEvent(ev);
    };

    const dispatchControlEnd = (originalEvent) => {
        this._controlStarted = false;
        const ev = createControlEvent('controlend', originalEvent, this._prevClientX, this._prevClientY);
        this.dispatchEvent(ev);
    };

    // This works as a static function. Because 'this' is not needed,
    // function() can be used instead of =>
    function createControlEvent(name, originalEvent, clientX, clientY) {
        const ev = new ControlEvent(name);
        Object.defineProperty(ev, 'target', { value: originalEvent.target });
        ev.originalEvent = originalEvent;
        ev.shiftKey = originalEvent.shiftKey;
        ev.ctrlKey = originalEvent.ctrlKey;
        ev.isInputMouse = originalEvent instanceof MouseEvent;
        ev.isInputWheel = originalEvent instanceof WheelEvent;
        ev.isInputTouch = (typeof TouchEvent !== 'undefined') && originalEvent instanceof TouchEvent;
        ev.clientX = clientY;
        ev.clientY = clientY;
        return ev;
    }

}


/**
 *  Support
 */

const ValueScale = {

    linear: {
        normalize: (val, min, max) => {
            return (val - min) / (max - min);
        },
        denormalize: (val, min, max) => {
            return min + (max - min) * val;
        }
    },

    log: {
        normalize: (val, min, max) => {
            min = Math.log(min);
            max = Math.log(max);
            return (Math.log(val) - min) / (max - min);
        },
        denormalize: (val, min, max) => {
            min = Math.log(min);
            max = Math.log(max);
            return Math.exp(min + (max - min) * val);
        }
    },

    db: {
        normalize: (val, min, max) => {
            return Math.pow(10.0, (val - max) / (max - min));
        },
        denormalize: (val, min, max) => {
            return max + (max - min) * Math.log10(val);
        }
    }

};


class ValueParser {

    static bool(s, def) {
        return ((s === 'true') || (s == 'false')) ? (s == 'true') : def;
    }

    static int(s, def) {
        const val = parseInt(s);
        return !isNaN(val) ? val : def;
    }

    static float(s, def) {
        const val = parseFloat(s);
        return !isNaN(val) ? val : def;
    }

    static string(s, def) {
        return s ? s : def;
    }

    static scale(s, def) {
        return ValueScale[s] ? ValueScale[s] : def;
    }

}


class SvgMath {

    // http://jsbin.com/quhujowota

    static describeArc(x, y, radius, startAngle, endAngle) {
        const start = this.polarToCartesian(x, y, radius, endAngle);
        const end = this.polarToCartesian(x, y, radius, startAngle);

        const largeArcFlag = endAngle - startAngle <= 180 ? '0' : '1';

        const d = [
            'M', start.x, start.y, 
            'A', radius, radius, 0, largeArcFlag, 0, end.x, end.y
        ].join(' ');

        return d;       
    }

    static polarToCartesian(centerX, centerY, radius, angleInDegrees) {
        const angleInRadians = (angleInDegrees - 90) * Math.PI / 180.0;

        return {
            x: centerX + (radius * Math.cos(angleInRadians)),
            y: centerY + (radius * Math.sin(angleInRadians))
        };
    }

}


// +------------------------------------------------------------------------+ //
// |                    CONCRETE WIDGET IMPLEMENTATIONS                     | //
// +------------------------------------------------------------------------+ //

class Knob extends RangeInputWidget {

    /**
     *  Internal
     */
    
    static get _unqualifiedNodeName() {
        return 'knob';
    }

    static get _attributeDescriptors() {
        return super._attributeDescriptors.concat([
            { key: 'sensibility', parser: ValueParser.float, default: 2.0 }
        ]);
    }

    static _initialize() {
        this._rangeStartAngle = -135;
        this._rangeEndAngle   =  135;

        this._svg = `<svg viewBox="0 0 100 100">
                       <g id="body">
                         <circle id="body" cx="50" cy="50" r="39"/>
                         <circle id="pointer" cx="50" cy="22" r="3.5"/>
                       </g>
                       <path id="range" fill="none" stroke-width="9"/>
                       <path id="value" fill="none" stroke-width="9"/>
                     </svg>`;
    }

    constructor() {
        super();

        this.addEventListener('controlstart', this._onGrab);
        this.addEventListener('controlcontinue', this._onMove);
        this.addEventListener('controlend', this._onRelease);
    }

    connectedCallback() {
        super.connectedCallback();

        this._root.innerHTML = `<style>
            #body { fill: ${this._style('--body-color', '#404040')}; }
            #range { stroke: ${this._style('--range-color', '#404040')}; }
            #value { stroke: ${this._style('--value-color', '#ffffff')}; }
        </style>`;

        const This = this.constructor;

        this._root.innerHTML += This._svg;
        this.style.display = 'block';
 
        const d = SvgMath.describeArc(50, 50, 45, This._rangeStartAngle, This._rangeEndAngle);
        this._root.getElementById('range').setAttribute('d', d);

        this._redraw();
    }
    
    _redraw() {
        const body = this._root.getElementById('body'),
              value = this._root.getElementById('value'),
              pointer = this._root.getElementById('pointer');

        if (!body) {
            return;
        }

        const This = this.constructor;
        const range = Math.abs(This._rangeStartAngle) + Math.abs(This._rangeEndAngle);
        const endAngle = This._rangeStartAngle + range * this._value;

        body.setAttribute('transform', `rotate(${endAngle}, 50, 50)`);
        value.setAttribute('d', SvgMath.describeArc(50, 50, 45, This._rangeStartAngle, endAngle));
        pointer.style.fill = this.value == 0 ? this._style('--pointer-off-color', '#000') 
                    : this._style('--pointer-on-color', window.getComputedStyle(value).stroke);
    }

    /**
     *  Private
     */

    _onGrab(ev) {
        this._startValue = this._value;
        this._dragDistance = 0;
        this._axisTracker = [];
    }

    _onMove(ev) {
        // Note: Relying on MouseEvent movementX/Y results in slow response when
        //       REAPER is configured to throotle down mouse events on macOS.
        //       Use custom deltaX/Y instead for such case.

        const dir = Math.abs(ev.deltaX) - Math.abs(ev.deltaY);

        this._axisTracker.push(dir);

        const axis = this._axisTracker.reduce((n0, n1) => n0 + n1);

        if (this._axisTracker.length > 20) {
            this._axisTracker.shift();
        }

        if (ev.isInputWheel) {
            document.body.style.cursor = axis > 0 ? 'ew-resize' : 'ns-resize';
        } else {
            document.body.style.cursor = 'none';
        }

        const dmov = (axis > 0 ? ev.deltaX : -ev.deltaY) / this.clientHeight;
        const k0 = 0.1;
        const k1 = 1.0 * (dmov < 0 ? -1 : 1);

        this._dragDistance += k0 * dmov + k1 * Math.pow(dmov, 2);

        const dval = this._dragDistance * this.props.sensibility;
        const val = Math.max(0, Math.min(1.0, this._startValue + dval));

        this._setNormalizedValueAndDispatchInputEventIfNeeded(val);
    }

    _onRelease(ev) {
        document.body.style.cursor = null;
    }

}


class Fader extends RangeInputWidget {

    /**
     *  Internal
     */
    
    static get _unqualifiedNodeName() {
        return 'fader';
    }

    static get _attributeDescriptors() {
        return super._attributeDescriptors.concat([
            { key: 'sensibility', parser: ValueParser.float, default: 10.0 }
        ]);
    }

    static _initialize() {
        this._svg = {
            LINES: `<svg width="100%" height="100%">
                    <rect id="body" width="100%" height="100%" />
                    <line id="value" y2="100%" stroke-width="100%" stroke-dasharray="7,1" />
                  </svg>`,
            LTR: `<svg width="100%" height="100%">
                    <line id="range" x1="5%" x2="5%" y2="100%" y1="0" stroke-width="7%" />
                    <line id="value" x1="5%" x2="5%" y2="100%" stroke-width="7%" />
                    <rect id="body" width="80%" height="100%" x="20%" />
                    <circle id="pointer" cx="60%" cy="20%" r="3.5" />
                  </svg>`,
            RTL: `<svg width="100%" height="100%">
                    <line id="range" x1="95%" x2="95%" y2="100%" y1="0" stroke-width="7%" />
                    <line id="value" x1="95%" x2="95%" y2="100%" stroke-width="7%" />
                    <rect id="body" width="80%" height="100%" />
                    <circle id="pointer" cx="40%" cy="20%" r="3.5" />
                  </svg>`
        };
    }

    constructor() {
        super();

        this.addEventListener('controlstart', this._onGrab);
        this.addEventListener('controlcontinue', this._onMove);
        this.addEventListener('controlend', this._onRelease);
    }

    connectedCallback() {
        super.connectedCallback();

        this._root.innerHTML = `<style>
            #body { fill: ${this._style('--body-color', '#404040')}; }
            #range { stroke: ${this._style('--range-color', '#404040')}; }
            #value { stroke: ${this._style('--value-color', '#ffffff')}; }
        </style>`;
        
        const This = this.constructor;

        switch (this._style('--graphic', 'lines').toLowerCase()) {
            case 'lines':
                this._root.innerHTML += This._svg.LINES;
                break;
            case 'split':
                this._root.innerHTML += this._style('direction', 'ltr') == 'ltr' ?
                                        This._svg.LTR : This._svg.RTL
                break;
            default:
                break;
        }

        this.style.display = 'block';
        this._redraw();
    }

    _redraw() {
        const body = this._root.getElementById('body'),
              value = this._root.getElementById('value'),
              pointer = this._root.getElementById('pointer');

        if (!body) {
            return;
        }

        const y = 100 * (1.0 - this.value);
        value.setAttribute('y1', `${y}%`);

        if (pointer) {
            pointer.style.fill = this.value == 0 ? this._style('--pointer-off-color', '#000') 
                        : this._style('--pointer-on-color', window.getComputedStyle(value).stroke);
            pointer.style.stroke = this._style('--pointer-border-color',
                                    window.getComputedStyle(body).fill);
            pointer.setAttribute('cy', `${y}%`);
        }
    }

    /**
     *  Private
     */

    _onGrab(ev) {
        if (ev.isInputWheel) {
            this._startValue = this._value;
            this._dragDistance = 0;
        } else {
            this._onMove(ev);
        }
    }

    _onMove(ev) {
        document.body.style.cursor = ev.isInputWheel ? 'ns-resize' : 'none';

        if (ev.isInputWheel) {
            this._dragDistance += -0.1 * ev.deltaY / this.clientHeight;

            const dval = this._dragDistance * this.props.sensibility;
            const val = Math.max(0, Math.min(1.0, this._startValue + dval));

            this._setNormalizedValueAndDispatchInputEventIfNeeded(val);
        } else {
            const y = (ev.clientY - this.getBoundingClientRect().top) / this.clientHeight;
            const val = 1.0 - Math.max(0, Math.min(1.0, y));

            this._setNormalizedValueAndDispatchInputEventIfNeeded(val);
        }
    }

    _onRelease(ev) {
        document.body.style.cursor = null;
    }

}


class Button extends InputWidget {

    /**
     *  Internal
     */
    
    static get _unqualifiedNodeName() {
        return 'button';
    }

    static get _attributeDescriptors() {
        return super._attributeDescriptors.concat([
            { key: 'value'    , parser: ValueParser.bool  , default: false       },
            { key: 'feedback' , parser: ValueParser.bool  , default: true        },
            { key: 'mode'     , parser: ValueParser.string, default: 'momentary' }
        ]);
    }

    constructor() {
        super();

        this.addEventListener('controlstart', this._onSelect);
        this.addEventListener('controlend', this._onUnselect);
    }

    connectedCallback() {
        super.connectedCallback();

        this._root.innerHTML = `<div style="
                                  width: 100%;
                                  height: 100%;
                                  display: flex;
                                  align-items: center;
                                  justify-content: center;
                                  cursor: default">
                                </div>`;

        // https://stackoverflow.com/questions/48498581/textcontent-empty-in-connectedcallback-of-a-custom-htmlelement
        let updating = false;
        const slot = document.createElement('slot');

        slot.addEventListener('slotchange', (ev) => {
            if (updating) {
                updating = false;
            } else {
                // Move <g-button> children to <div>
                updating = true;
                const div = this._root.querySelector('div');
                div.innerHTML = '';
                for (let node of ev.target.assignedNodes()) {
                    div.appendChild(node.cloneNode(true));
                    this.removeChild(node);
                }
            }
        });

        this._root.appendChild(slot);
        this.style.display = 'inline-block';

        this.reset();
    }

    reset() {
        // Reset inline styles
        this.style.color = '';
        this.style.borderColor = '';
        this.style.backgroundColor = '';

        // Read computed styles
        this._color = this._style('color', /*inherited*/);
        this._backgroundColor = this._style('background-color' /*rgb(0,0,0)*/);
        this._borderColor = this._style('border-color' /*rgb(0,0,0)*/);
        this._selectedColor = this._style('--selected-color', '#000');

        this._redraw();
    }

    _redraw() {
        if (! this._props['feedback']) {
            return;
        }
        
        if (this.value) {
            this.style.color = this._selectedColor;
            this.style.borderColor = this._color;
            this.style.backgroundColor = this._color;
        } else {
            this.style.color = this._color;
            this.style.borderColor = this._borderColor;
            this.style.backgroundColor = this._backgroundColor;
        }
    }

    /**
     *  Private
     */

    _onSelect(ev) {
        if (ev.isInputWheel) {
            return;
        }

        if (this.props.mode == 'momentary') {
            this._setValueAndDispatchInputEventIfNeeded(true);
        } else if (this.props.mode == 'latch') {
            this._setValueAndDispatchInputEventIfNeeded(! this.value);
        }
    }

    _onUnselect(ev) {
        if (ev.isInputWheel) {
            return;
        }

        if (this.props.mode == 'momentary') {
            this._setValueAndDispatchInputEventIfNeeded(false);
        }
    }

}


class ResizeHandle extends InputWidget {

    /**
     *  Internal
     */

    static get _unqualifiedNodeName() {
        return 'resize';
    }

    static get _attributeDescriptors() {
        return super._attributeDescriptors.concat([
            { key: 'minWidth'       , parser: ValueParser.int  , default: 100   },
            { key: 'minHeight'      , parser: ValueParser.int  , default: 100   },
            { key: 'maxWidth'       , parser: ValueParser.int  , default: 0     },
            { key: 'maxHeight'      , parser: ValueParser.int  , default: 0     },
            { key: 'maxScale'       , parser: ValueParser.float, default: 2     },
            { key: 'keepAspectRatio', parser: ValueParser.bool , default: false },
        ]);
    }

    static _initialize() {
        this._svg = {
            LINES_1: `<svg viewBox="0 0 100 100">
                        <line x1="95" y1="45" x2="45" y2="95" stroke-width="3"/>
                        <line x1="70" y1="95" x2="95" y2="70" stroke-width="3"/>
                      </svg>`,
            LINES_2: `<svg viewBox="0 0 100 100">
                        <line x1="0" y1="100" x2="100" y2="0"/>
                        <line x1="100" y1="25" x2="25" y2="100"/>
                        <line x1="50" y1="100" x2="100" y2="50"/>
                        <line x1="75" y1="100" x2="100" y2="75"/>
                      </svg>`,
            DOTS: `<svg viewBox="0 0 100 100">
                     <path d="M80.5,75.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                       C78.262,70.5,80.5,72.74,80.5,75.499z"/>
                     <path d="M50.5,75.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                       C48.262,70.5,50.5,72.74,50.5,75.499z"/>
                     <path d="M80.5,45.499c0,2.763-2.238,5.001-5,5.001c-2.761,0-5-2.238-5-5.001c0-2.759,2.239-4.999,5-4.999
                       C78.262,40.5,80.5,42.74,80.5,45.499z"/>
                   </svg>`
        };
    }

    constructor() {
        super();
        
        this._width = 0;
        this._height = 0;

        this.addEventListener('controlstart', this._onGrab);
        this.addEventListener('controlcontinue', this._onDrag);

        const unsetCursorIfNeeded = (ev) => {
            if (!this.isControlStarted && !this._isMouseIn) {
                document.body.style.cursor = null;
            }
        };

        this.addEventListener('controlend', unsetCursorIfNeeded);

        this.addEventListener('mouseenter', (ev) => {
            this._isMouseIn = true;
            document.body.style.cursor = 'nwse-resize';
        });

        this.addEventListener('mouseleave', () => {
            this._isMouseIn = false;
            unsetCursorIfNeeded();
        });
    }

    connectedCallback() {
        super.connectedCallback();

        this.style.position = 'absolute';
        this.style.zIndex = '100';
        this.style.right = '0px';
        this.style.bottom = '0px';

        if (parseInt(this._style('width')) == 0) {
            this.style.width = '24px';
        }

        if (parseInt(this._style('height')) == 0) {
            this.style.height = '24px';
        }

        const color = this._style('--color', '#000');

        this._root.innerHTML = `<style>
            path { fill: ${color}; }
            line { stroke: ${color}; }
        </style>`;

        const svg = this.constructor._svg;

        switch (this._style('--graphic', 'lines').toLowerCase()) {
            case 'lines':
            case 'lines-1':
                this._root.innerHTML += svg.LINES_1;
                break;
            case 'lines-2':
                this._root.innerHTML += svg.LINES_2;
                break;
            case 'dots':
                this._root.innerHTML += svg.DOTS;
                break;
            default:
                break;
        }
    }

    _propertyUpdated(key, value) {
        super._propertyUpdated(key, value);

        if (this.props.maxScale > 0) {
            this.props.maxWidth = this.props.maxScale * this.props.minWidth;
            this.props.maxHeight = this.props.maxScale * this.props.minHeight;
        }

        this._aspectRatio = this.props.minWidth / this.props.minHeight;
    }

    /**
     *  Private
     */

    _onGrab(ev) {
        this._width = this.parentNode.clientWidth;
        this._height = this.parentNode.clientHeight;
    }

    _onDrag(ev) {
        // Note 1: Relying on MouseEvent movementX/Y results in slow response
        //         when REAPER is configured to throotle down mouse events on
        //         macOS. Use custom deltaX/Y instead for such case.
        //         https://www.reddit.com/r/Reaper/comments/rsnjyp/just_found_fix_for_all_reaper_lag_low_fps_on_macos/
        // Note 2: On Windows touchmove events stop triggering if the window is
        //         resized while the listener runs. mousemove not affected.

        const useMouseDelta = /mac/i.test(window.navigator.platform) && ev.isInputMouse;
        const deltaX = useMouseDelta ? ev.originalEvent.movementX : ev.deltaX;
        const deltaY = useMouseDelta ? ev.originalEvent.movementY : ev.deltaY;
        
        let newWidth = this._width + deltaX;
        newWidth = Math.max(this.props.minWidth, Math.min(this.props.maxWidth, newWidth));

        let newHeight = this._height + deltaY;
        newHeight = Math.max(this.props.minHeight, Math.min(this.props.maxHeight, newHeight));

        if (this.props.keepAspectRatio) {
            if (deltaX > deltaY) {
                newHeight = newWidth / this._aspectRatio;
            } else {
                newWidth = newHeight * this._aspectRatio;
            }
        }

        if ((this._width != newWidth) || (this._height != newHeight)) {
            this._width = newWidth;
            this._height = newHeight;

            this._setValueAndDispatchInputEventIfNeeded({
                width: this._width,
                height: this._height
            });
        }
    }

}


/**
 *  Static library initialization
 */

window.Guinda = { Knob, Fader, Button, ResizeHandle };
Object.values(window.Guinda).forEach((cls) => cls.defineCustomElement());

})();
