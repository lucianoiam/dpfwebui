Apices - Audio Plugins In C++ & ES6
-----------------------------------

Raw power meets versatility.

Apices is a library for creating audio plugins with potentially complex user
interfaces, without wasting precious CPU cycles in the critical realtime audio
processing stage. Plugins implement a web view for running the UI, which
interacts with the audio host through a small factory shipped JavaScript class.

![](https://user-images.githubusercontent.com/930494/124803158-0db54900-df59-11eb-8c69-4bb3369d54f2.png)

*WebGain example running on Bitwig. A real world plugin is under development [here](https://github.com/lucianoiam/castello-rev).*

****

### Features

* Based on DISTRHO Plugin Framework
* C++ for DSP code
* HTML / CSS / JS for UI development
* VST2 / LV2 plugin formats and JACK standalone app (BETA)
* Linux / Mac / Windows
* Just the powerful basics
* BSD-like license

### Example UI code

```JavaScript
class MyPluginWebUI extends DISTRHO_WebUI {

    constructor() {
    	super();
    
        // Connect <input type="range" id="gain"> element to a parameter

        document.getElementById('gain').addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });

        this.flushInitMessageQueue();
    }

    parameterChanged(index, value) {
        // Host informs a parameter change, update input element value

        switch (index) {
            case 0:
                document.getElementById('gain').value = value;
                break;
            default:
                break;
        }
    }
    
}
```

**Usage of JS frameworks is up to the developer**

More information [here](https://github.com/lucianoiam/apices/blob/master/doc/internals.md)

### About DISTRHO Plugin Framework (DPF)

The framework sports an accurate motto: "DPF is designed to make development of
new plugins an easy and enjoyable task". It is a low footprint yet powerful tool
that allows developers to focus on productivity by already solving many not
trivial issues commonly found in the audio plugins domain.

What makes it great?

- It comes with a clear permissive license
- Bloat-free: scope is limited to audio plugins
- Makefile based, MinGW compatible
- Extremely low learning curve
- High quality clean C++ codebase
- Great experienced community around

Its full documentation and code can be found at https://github.com/DISTRHO/DPF,
this repo includes it as a git submodule in `dpf`.  Do not forget to visit
https://github.com/DISTRHO for many other cool audio projects.

If you find libre software useful please support the developers
