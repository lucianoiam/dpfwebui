Very rough code for embedding a web view in a DPF plugin
--------------------------------------------------------

Goal is to develop multiplatform audio plugins with web-based UIs based on the
minimalistic DISTRHO Plugin Framework

* DPF project page: https://github.com/DISTRHO/DPF

This is early work in progress...


Architecture

* On Linux launch a separate process running WebKitGTK then reparent window
* On macOS use a Cocoa web view
* On Windows use the Chromium based WebView2 provided by the OS
