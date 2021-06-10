Multiplatform VST audio plugins with HTML/JavaScript UI
-------------------------------------------------------

An alternate approach to plugin graphics

* Based on DPF: DISTRHO Plugin Framework
* C++ for DSP code
* HTML/CSS/JS for UI code
* Linux, macOS and Windows
* VST2, LV2 and JACK standalone app
* Lightweight, just the (powerful) basics
* Makefile based
* BSD-like license

Example code:

```JavaScript
class WebExampleUI extends WebUI {

    constructor() {
    	super();
    
        // Change a parameter...
        this.setParameterValue(1, 2.3);
    }

    parameterChanged(index, value) {
    
        // ...a parameter has changed
        
    }
    
}
```

This is alpha software, a working plugin will be released soon to demo its capabilities.

**"DPF is designed to make development of new plugins an easy and enjoyable task"**

Do not forget to visit https://github.com/DISTRHO

If you find libre software useful please support the developers


![](https://user-images.githubusercontent.com/930494/121346399-595adf80-c926-11eb-9131-3269de4398b7.png)
