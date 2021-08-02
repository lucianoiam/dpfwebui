### Dependencies

Usage of JS frameworks is up to the developer. No web equivalent versions of the
DPF/DGL widgets are provided. There are some options available:

- Rely on stock HTML elements plus styling. This is not recommended but possible.
- Browse the web for available toolkits like this one [here](https://github.com/DeutscheSoft/toolkit)
- Try my widgets library called [Guinda](https://github.com/lucianoiam/guinda). It
is incomplete and under heavy development as of Jun '21.
- Roll your own widgets. HTML5 offers plenty of tools, being SVG and canvas
worth looking into. Even a quick combination of images, stylesheets and little
code can do the job.

### Web view implementation

- On Linux a WebKitGTK web view instance runs in a separate process to keep the
plugin GTK-free.

- On macOS WKWebView is used.

- On Windows Edge WebView2 is used. As of Jun '21 the end user needs to install a
runtime library https://developer.microsoft.com/microsoft-edge/webview2. It is
an official library from MS and expected to become bundled into Windows at some
point.

### Integration with the underlying C++ framework (DPF)

A small JS wrapper around the C++ `DISTRHO::UI` class is provided for convenience.
New integrations between C++ and JS code can be easily built leveraging the
standard `postMessage()` call. For Linux and Mac a message handler called `host`
is installed, thus providing access to a native callback through the function:

`window.webkit.messageHandlers.host.postMessage()`

On Windows WebView2 already provides a single, fixed, built-in entry point to
native:

`window.chrome.webview.postMessage()`

In all cases the `postMessage()` function is mapped to a custom symbol for
hiding the differences between platforms:

`window.webviewHost.postMessage()`

In an attempt to keep the interface symmetrical and generic, `window.webviewHost`
is created as a `EventTarget` instance that can listened for events named
'message' on the JS side. This allows C++ to send messages by running the
following JS code:

`window.webviewHost.dispatchEvent(new CustomEvent('message',{detail:args}))`

The `DISTRHO::WebHostUI` and JS `DISTRHO_UI` classes use the above mechanism
to map some useful plugin methods, like the ones shown in the first code example
of the main README.

The bridge interface in a nutshell:

```
// Send from JS to C++

window.webviewHost.postMessage([...]);

void WebHostUI::webMessageReceived(const JsValueVector&) {

   // Receive in C++ from JS

}

// Send from C++ to JS

WebHostUI::webPostMessage({...});

window.webviewHost.addMessageListener((args) => {
    
    // Receive in JS from C++

});
```

Message arguments must be an array/vector containing values of primitive data
types. These values are wrapped by `DISTRHO::JsValue` instances. The following
JS types are supported: boolean, number, string. Any other types are mapped to
null.
