Early code for embedding a web view in a DPF plugin
---------------------------------------------------

Supersedes https://github.com/lucianoiam/dpf-cef

Goal is to enable developing multiplatform audio plugins with web-based UIs
based on the lightweight and libre DISTRHO Plugin Framework https://github.com/DISTRHO/DPF

This is early work in progress...


Proposed implementation

* On Linux launch a separate process running WebKitGTK then reparent window
* On macOS embed a Cocoa web view
* On Windows embed a Chromium-based WebView2 provided by the OS


![](https://user-images.githubusercontent.com/930494/118394103-eae67280-b642-11eb-8e98-66267542adc0.png)
