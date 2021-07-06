Audio plugins featuring webview based UIs
-----------------------------------------

An alternate approach to plugin user interfaces.

![](https://user-images.githubusercontent.com/930494/122536098-77aa9480-d024-11eb-93a4-4d4174c6559c.png)

*WebGain example running on Bitwig. A real world plugin is under development [here](https://github.com/lucianoiam/castello-rev).*

****

### Features

* Based on DPF: DISTRHO Plugin Framework
* C++ for DSP code
* HTML/CSS/JS for UI development
* VST2/LV2 plugin formats and JACK standalone app
* Linux/Mac/Windows
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

More information [here](https://github.com/lucianoiam/dpf-webui/blob/master/doc/internals.md)

### About DPF — DISTRHO Plugin Framework

The framework sports an accurate motto: "DPF is designed to make development of
new plugins an easy and enjoyable task". It is a low footprint yet powerful tool
that allows developers to focus on productivity by already solving many not
trivial issues commonly found in the audio plugins domain.

What makes it great?

- It comes with a clear libre license
- Zero bloat: scope is limited to audio plugins
- Makefile based, MinGW compatible
- Absurdingly low learning curve
- High quality clean C++ codebase
- Great experienced community around

Its full documentation and code can be found at https://github.com/DISTRHO/DPF,
this repo includes it as a git submodule in `lib/DPF`. And do not forget to
visit https://github.com/DISTRHO for many other cool audio projects.

If you find libre software useful please support the developers
