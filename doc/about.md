- On Linux a WebKitGTK or Chromium Embedded Framework (CEF) web view instance
  runs in a child process.

- On macOS the system WKWebView is used.

- On Windows Edge WebView2 is used. Windows <= 10 users must first install a
runtime library from https://developer.microsoft.com/microsoft-edge/webview2.
Windows 11 already ships with this library.

Usage of JS frameworks is up to the developer. No web equivalent versions of the
DPF/DGL widgets are provided. These are some available solutions:

- Look for libraries, for example [here](https://github.com/lucianoiam/guinda) and [here](https://github.com/DeutscheSoft/toolkit)
- Rely on stock HTML elements plus styling
- Create custom widgets, HTML offers plenty of tools like SVG, canvas and bitmap
images.
