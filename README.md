Very rough code for embedding a web view in a DPF plugin
--------------------------------------------------------

Supersedes https://github.com/lucianoiam/dpf-cef

Goal is to develop multiplatform audio plugins with web-based UIs based on the
minimalistic DISTRHO Plugin Framework

* DPF project page: https://github.com/DISTRHO/DPF

This is early work in progress...


Proposed implementation

* On Linux launch a separate process running WebKitGTK then reparent window
* On macOS embed a Cocoa web view
* On Windows embed a Chromium-based WebView2 provided by the OS
