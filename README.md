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
    
        // Change a parameter...
        this.setParameterValue(1, 2.3);
    }

    parameterChanged(index, value) {
    
        // ...a parameter has changed
        
    }
    
}
```

### Dependencies

Only the system web view. Usage of JS frameworks is up to the developer.

- On Linux a WebKitGTK web view instance runs in a separate process to keep the
plugin free of references to libgtk. Note that this approach relies on XEmbed,
which is not supported by Wayland. On such platform the plugin window will
appear detached.

- On macOS WKWebView is used.

- On Windows Edge WebView2 is used. As of Jun '21 the end user needs to install a
runtime library https://developer.microsoft.com/microsoft-edge/webview2. It is
an official library from MS and expected to become bundled into Windows at some
point.

No web equivalent versions of the DPF/DGL widgets are provided. There are some
options available:

- Rely on stock HTML elements plus styling. This is not recommended but possible.
- Browse the web for available toolkits like this one [here](https://github.com/DeutscheSoft/toolkit)
- Try my widgets library called [Awww](https://github.com/lucianoiam/awww). It is
incomplete and under heavy development as of Jun '21.
- Roll your own widgets. HTML5 offers a pletora of tools, being SVG and canvas
worth looking into. Even a quick combination of images, stylesheets and little
code can do the job.

### Integration with the underlying C++ framework (DPF)

A small JS wrapper around the C++ `DISTRHO::UI` class is provided for convenience.
New integrations between C++ and JS code can be easily built leveraging the
standard `postMessage()` call. For Linux and Mac a message handler called `host`
is installed, thus providing access to a native callback through the function:

`window.webkit.messageHandlers.host.postMessage()`

On Windows, WebView2 already provides a single built-in entry point to native:

`window.chrome.webview.postMessage()`

In all cases the `postMessage()` function is mapped to a custom symbol for
hiding the differences between platforms:

`window.webviewHost.postMessage()`

In an attempt to keep the interface symmetrical and generic, `window.webviewHost`
is created as a `EventTarget` instance that can listened for events named
'message' on the JS side. This allows C++ to send messages by calling:

`window.webviewHost.dispatchEvent(new CustomEvent('message',{detail:args}))`

The `DISTRHO::ProxyWebUI` and JS `DISTRHO_WebUI` classes use the above mechanism
to map some useful plugin methods, like the ones shown in the first code example
of this README.

The bridge interface in a nutshell:

```
// Send ( js → cpp )

window.webviewHost.postMessage([...]);

void ProxyWebUI::webMessageReceived(const ScriptValueVector&) {

   // Receive

}

// Send ( cpp → js )

ProxyWebUI::webPostMessage({...});

window.webviewHost.addMessageListener((args) => {
    
    // Receive

});
```

The C++ class carries the "Proxy" moniker because it merely acts as a bridge
between the two languages and the actual UI functionality is meant to be
implemented in JavaScript.

Message arguments must be an array/vector containing values of primitive data
types. These values are wrapped by `DISTRHO::ScriptValue` instances. The
following JS types are supported: boolean, number, string. Any other types are
mapped to null.

### Note on the Makefile

There are various bits of Makefiles. Derived projects only need to tweak the
main file, ie. the one called Makefile without extension in the repo root.
If advanced tweaks are needed here is the include order:

```
Makefile
Makefile.plugins.mk
Makefile.base.mk
lib/DPF/Makefile.plugins.mk
lib/DPF/Makefile.base.mk
Makefile.support.mk
```

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
