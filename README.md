Multiplatform VST audio plugins with HTML/JavaScript UI
-------------------------------------------------------

An alternate approach to plugin graphics.

* Based on DPF: DISTRHO Plugin Framework
* C++ for DSP code
* HTML/CSS/JS for UI code
* Targets Linux, macOS and Windows
* Generates VST2 / LV2 plugins and JACK standalone app
* Lightweight, just the powerful basics
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

**"DPF is designed to make development of new plugins an easy and enjoyable task"**

Do not forget to visit https://github.com/DISTRHO

![](https://user-images.githubusercontent.com/930494/121346399-595adf80-c926-11eb-9131-3269de4398b7.png)

If you find libre software useful please support the developers

