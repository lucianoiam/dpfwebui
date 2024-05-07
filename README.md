Web User Interfaces for DPF
---------------------------

This project builds on top of the [DPF](http://github.com/DISTRHO/DPF) audio
plugin framework to add web-based UI support. Plugins can leverage JavaScript
and related tech to provide complex user interfaces on the computer running
the plugin and optionally over the local network.

![Screenshot_2022-04-09_13-47-34](https://user-images.githubusercontent.com/930494/162572881-cba8857c-c4d2-444f-8b10-ab27ba86ea30.png)

*Examples running on Bitwig for Linux*

****

### Features

* Based on DISTRHO Plugin Framework (DPF)
* C++ for DSP development
* WebKitGTK or CEF on Linux, WKWebView on macOS, Edge WebView2 on Windows
* VST3 / VST2 / CLAP / LV2 plugin formats
* Network UI support, eg. for remote control using a tablet
* Just the powerful basics

### Example JavaScript UI code

```JavaScript
class ExampleUI extends DISTRHO.UI {

    constructor() {
        super();
    
        // Connect <input type="range" id="gain"> element to a parameter

        document.getElementById('gain').addEventListener('input', (ev) => {
            this.setParameterValue(0, parseFloat(ev.target.value));
        });
    }

    parameterChanged(index, value) {
        // Host informs a parameter change, update input element value

        switch (index) {
            case 0:
                document.getElementById('gain').value = value;
                break;
        }
    }
    
}
```

The complete UI interface is defined [here](https://github.com/lucianoiam/dpfwebui/blob/master/webui/src/ui/dpf.js).

### Plugin implementations

[Consul](https://github.com/lucianoiam/consul) \
[Castello Reverb](https://github.com/lucianoiam/castello)

### Related projects

[Guinda](https://github.com/lucianoiam/guinda) \
[Pisco](https://github.com/lucianoiam/pisco)

