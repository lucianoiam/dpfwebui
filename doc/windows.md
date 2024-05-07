NOTES FOR WINDOWS
-----------------

1. Native binaries can be built by cross-compiling on Linux or by installing the
   MinGW toolchain on Windows. For the latter option first download MSYS2 from
   https://www.msys2.org/ and then follow website instructions on how to install
   the mingw-64 GCC.


2. For building the plugin the Edge WebView2 SDK is required. Makefile will try
   to download the SDK if the NuGet tool is already present. If not, NuGet will
   be automatically downloaded for MinGW but on Linux that could need extra work
   https://docs.microsoft.com/en-us/nuget/install-nuget-client-tools
   Some distros like Ubuntu already offer it: sudo apt install nuget

   Running MinGW on Windows 7 can cause nuget.exe to fail due to a TLS error, in
   such case here is the fix: https://github.com/NuGet/NuGetGallery/issues/8176

   The SDK can also be downloaded using the free Visual Studio Community IDE:

   - Create Project
   - Right click on solution
   - Manage NuGet packages
   - Find and install Microsoft.Web.WebView2
   - Copy < SOLUTION_DIR >/packages/Microsoft.Web.WebView2.< VERSION > to
     < PLUGIN_DIR >/lib/Microsoft.Web.WebView2  (version suffix stripped)


3. For running the plugin on Windows 10 or earlier, the Microsoft Edge WebView2
   Runtime must be installed https://developer.microsoft.com/microsoft-edge/webview2
