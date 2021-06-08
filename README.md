[WIP] Multiplatform VST audio plugins with HTML/JavaScript UI
-------------------------------------------------------------

What if the UI toolkit was a web UI toolkit?

* Based on DPF: DISTRHO Plugin Framework
* C++ for DSP code
* HTML/CSS/JS for UI code
* Linux, macOS and Windows
* VST2, LV2, DSSI and JACK standalone app
* Lightweight, just the (powerful) basics
* Makefile based
* BSD-like license

Example code (unstable API):

```JavaScript
class WebExampleUI extends WebUI {

    setParameterValue(index, value) {
        // Change a parameter...
        super.setParameterValue(index, value);
    }

    parameterChanged(index, value) {
        // ...a parameter has changed
    }
    
}
```

This is work in progress ( nearly 80% complete )

**"DPF is designed to make development of new plugins an easy and enjoyable task"**

Do not forget to visit https://github.com/DISTRHO

If you find libre software useful please support the developers


![](https://user-images.githubusercontent.com/930494/121150965-42dc5780-c844-11eb-88ff-384728b017f7.png)
