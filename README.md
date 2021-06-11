Audio plugins featuring webview based UIs
-----------------------------------------

An alternate approach to plugin graphics.

* Based on DPF: DISTRHO Plugin Framework
* C++ for DSP code
* HTML/CSS/JS for UI development
* VST2 / LV2 plugin formats and JACK standalone app
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

JavaScript and rendering are provided by system libraries:

- On Linux a WebKitGTK web view instance runs on separate process to keep the
plugin free of references to libgtk. Note that this approach relies on XEmbed,
which is not supported by Wayland. On that platform the plugin window will
appear detached.

- On macOS WKWebView is used.

- On Windows Edge WebView2 is used. This needs installing a runtime on the user
computer that is likely to become bundled into Windows soon:

https://developer.microsoft.com/microsoft-edge/webview2

Usage of JS frameworks is up to the developer. A small JS wrapper around the C++
`DISTRHO::UI` class is provided for convenience. New integrations between C++
and JS code can be easily built leveraging the standard `postMessage()` call.
For Linux and Mac a message handler called `host` is installed, thus providing
access to a native callback through this function:

`window.webkit.messageHandlers.host.postMessage()`

On Windows, WebView2 already provides a single built-in entry point to native:

`window.chrome.webview.postMessage()`

In both cases the `postMessage()` function is mapped by `DISTRHO::BaseWebView`
to a custom symbol for hiding the differences between platforms:

`window.webviewHost.postMessage()`

In an attempt to keep the interface symmetrical and generic, `window.webviewHost`
is created as a `EventTarget` instance that can listened for events named
'message' on the JS side. This allows C++ to send messages by calling:

`window.webviewHost.dispatchEvent(new CustomEvent('message',{detail:args}))`

The `DISTRHO::WebUI` and JS `WebUI` classes use the above to map some useful
plugin methods like shown in the example code above.

Summarizing:

```
// Send

window.webviewHost.postMessage([...]);

void WebUI::webMessageReceived(const ScriptValueVector&) {

   // Receive

}

// Send

WebUI::webPostMessage({...});

window.webviewHost.addMessageListener((args) => {
    
    // Receive

});
```

The message arguments must be an array/vector containing values with primitive
data types. These values are wrapped in `DISTRHO::ScriptValue` instances. The
following JS types are supported: boolan, number, string. Any other types are
mapped to null.

For the available C++ interfaces check https://github.com/DISTRHO/DPF and its
plugin examples.

**"DPF is designed to make development of new plugins an easy and enjoyable task"**

Do not forget to visit https://github.com/DISTRHO for many other cool audio
projects.

![](https://user-images.githubusercontent.com/930494/121346399-595adf80-c926-11eb-9131-3269de4398b7.png)

If you find libre software useful please support the developers
