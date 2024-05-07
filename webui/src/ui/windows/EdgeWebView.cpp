/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "EdgeWebView.hpp"

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <winuser.h>

#include "extra/macro.h"
#include "extra/Path.hpp"
#include "Variant.hpp"

#include "DistrhoPluginInfo.h"

#define WEBVIEW2_DOWNLOAD_URL "https://developer.microsoft.com/en-us/microsoft-edge/webview2/#download-section"

#define JS_POST_MESSAGE_SHIM  "window.host.postMessage = (payload) => window.chrome.webview.postMessage(payload);"

#define WSTR_CONVERTER std::wstring_convert<std::codecvt_utf8<wchar_t>>()
#define TO_LPCWSTR(s)  WSTR_CONVERTER.from_bytes(s).c_str()
#define TO_LPCSTR(s)   WSTR_CONVERTER.to_bytes(s).c_str()
#define WXSTR(s)       L"" XSTR(s)

LRESULT CALLBACK HelperWindowProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardFilterProc(int nCode, WPARAM wParam, LPARAM lParam);

USE_NAMESPACE_DISTRHO

EdgeWebView::EdgeWebView(String userAgentComponent)
    : fUserAgent(userAgentComponent)
    , fHelperClassName(nullptr)
    , fHelperHwnd(0)
    , fKeyboardHook(0)
    , fReady(false)
    , fHandler(nullptr)
    , fController(nullptr)
    , fView(nullptr)
{
    String dllPath = Path::getPluginLibrary() + "\\" + "WebView2Loader.dll";
    const HMODULE hm = LoadLibrary(TO_LPCWSTR(dllPath));

    if (hm == 0) {
        errorMessageBox(L"Could not load WebView2Loader.dll");
        return;
    }

    setBackgroundColor(0); // transparent

    constexpr size_t maxClassName = 256;
    fHelperClassName = new WCHAR[maxClassName];
    swprintf(fHelperClassName, maxClassName, L"EdgeWebView_%s_%d", XSTR(DPF_WEBUI_PROJECT_ID_HASH),
        std::rand());
    WNDCLASSEX helperClass;
    ZeroMemory(&helperClass, sizeof(helperClass));
    helperClass.cbSize = sizeof(helperClass);
    helperClass.cbWndExtra = 2 * sizeof(LONG_PTR);
    helperClass.lpszClassName = fHelperClassName;
    helperClass.lpfnWndProc = HelperWindowProc;
    RegisterClassEx(&helperClass);

    fHelperHwnd = CreateWindowEx(0, helperClass.lpszClassName, L"EdgeWebView Helper",
                                    WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
    SetWindowLongPtr(fHelperHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(fHelperHwnd, SW_SHOW);

    // Unfortunately there is no Edge WebView2 API for disabling keyboard input
    // https://github.com/MicrosoftEdge/WebView2Feedback/issues/112
    fKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardFilterProc, GetModuleHandle(0), 0);

    fHandler = new WeakWebView2EventHandler(this);

    // These requests are queued until Edge WebView2 initializes itself
    injectHostObjectScripts();
    String js = String(JS_POST_MESSAGE_SHIM);
    injectScript(js);  

    // Initialize Edge WebView2. Avoid error: Make sure COM is initialized - 0x800401F0 CO_E_NOTINITIALIZED
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    typedef (*PFN_CreateCoreWebView2EnvironmentWithOptions)(PCWSTR browserExecutableFolder, PCWSTR userDataFolder,
        ICoreWebView2EnvironmentOptions* environmentOptions,
        ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler);

# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
    const PFN_CreateCoreWebView2EnvironmentWithOptions pfnCreateCoreWebView2EnvironmentWithOptions
        = (PFN_CreateCoreWebView2EnvironmentWithOptions)GetProcAddress(hm, "CreateCoreWebView2EnvironmentWithOptions");
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif

    const HRESULT result = pfnCreateCoreWebView2EnvironmentWithOptions(0, TO_LPCWSTR(Path::getUserData()),
        0, fHandler);

    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
    }

    FreeLibrary(hm);
}

EdgeWebView::~EdgeWebView()
{
    if (fHandler != nullptr) {
        fHandler->release();
    }

    if (fKeyboardHook != 0) {
        UnhookWindowsHookEx(fKeyboardHook);
    }

    if (fController != nullptr) {
        ICoreWebView2Controller2_Close(fController);
        ICoreWebView2Controller2_Release(fController);
    }

    if (fHelperHwnd != 0) {
        DestroyWindow(fHelperHwnd);
    }

    if (fHelperClassName != nullptr) {
        UnregisterClass(fHelperClassName, 0);
        delete[] fHelperClassName;
    }
}

float EdgeWebView::getMonitorScaleFactor(HWND hWnd)
{
    float k = 1.f;

    const HMODULE shcore = LoadLibraryA("Shcore.dll");
    if (shcore == nullptr) {
        return k;
    }

    typedef HRESULT (*PFN_GetProcessDpiAwareness)(HANDLE hProc, PROCESS_DPI_AWARENESS *pValue);
    typedef HRESULT (*PFN_GetDpiForMonitor)(HMONITOR hMon, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);

# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
    const PFN_GetProcessDpiAwareness GetProcessDpiAwareness
        = (PFN_GetProcessDpiAwareness)GetProcAddress(shcore, "GetProcessDpiAwareness");
    const PFN_GetDpiForMonitor GetDpiForMonitor
        = (PFN_GetDpiForMonitor)GetProcAddress(shcore, "GetDpiForMonitor");
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif

    PROCESS_DPI_AWARENESS dpiAware;

    if ((GetProcessDpiAwareness != nullptr) && (GetDpiForMonitor != nullptr)
            && (SUCCEEDED(GetProcessDpiAwareness(0, &dpiAware)))
            && (dpiAware != PROCESS_DPI_UNAWARE)) {
        HMONITOR hMon;
        if (hWnd == 0) {
            hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
        } else {
            hMon = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
        }

        // GetScaleFactorForMonitor() can return incorrect values in some cases,
        // for example it is 1.4 when running a plugin on REAPER with display
        // scaling set to 150%. Same plugin on Ableton Live correctly reads 1.5.
        // https://stackoverflow.com/questions/63692872/is-getscalefactorformonitor-winapi-returning-incorrect-scaling-factor
        /*DEVICE_SCALE_FACTOR scaleFactor;
        if (SUCCEEDED(GetScaleFactorForMonitor(hMon, &scaleFactor))) {
            k = static_cast<float>(scaleFactor) / 100.f;
        }*/

        UINT dpiX, dpiY;
        if (SUCCEEDED(GetDpiForMonitor(hMon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
            k = static_cast<float>(dpiX) / 96.f;
        }
    }

    FreeLibrary(shcore);

    return k;
}

float EdgeWebView::getDevicePixelRatio()
{
    // ICoreWebView2Controller3_get_RasterizationScale() always returns 1.
    // Using it would also require to add a view-ready callback to update the
    // web view and plugin UI sizes because WebView2 initialization is async,
    // creating a visual glitch that is less desirable than relying on an
    // imperfect method for determining scaling factor (but at least sync).
    return getMonitorScaleFactor(reinterpret_cast<HWND>(getParent()));

}

void EdgeWebView::realize()
{
    SetParent(fHelperHwnd, reinterpret_cast<HWND>(getParent()));
    RedrawWindow(fHelperHwnd, 0, 0, RDW_ERASE);
}

void EdgeWebView::navigate(String& url)
{
    if (fView == nullptr) {
        fUrl = url;
    } else {
        ICoreWebView2_Navigate(fView, TO_LPCWSTR(url));
    }
}

void EdgeWebView::runScript(String& source)
{
    if ((fView == nullptr) || ! fReady) {
        return;
    }

    ICoreWebView2_ExecuteScript(fView, TO_LPCWSTR(source), 0);
}

void EdgeWebView::injectScript(String& source)
{
    if (fController == nullptr) {
        fInjectedScripts.push_back(source);
    } else {
        ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, TO_LPCWSTR(source), 0);
    }
}

void EdgeWebView::onSize(uint width, uint height)
{
    SetWindowPos(fHelperHwnd, 0, 0, 0, width, height, SWP_NOOWNERZORDER | SWP_NOMOVE);

    if (fController != nullptr) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = static_cast<LONG>(width);
        bounds.bottom = static_cast<LONG>(height);

        ICoreWebView2Controller2_put_Bounds(fController, bounds);
    }
}

HRESULT EdgeWebView::handleWebView2EnvironmentCompleted(HRESULT result,
                                                        ICoreWebView2Environment* environment)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }

    ICoreWebView2Environment_CreateCoreWebView2Controller(environment, fHelperHwnd, fHandler);

    return S_OK;
}

HRESULT EdgeWebView::handleWebView2ControllerCompleted(HRESULT result,
                                                       ICoreWebView2Controller* controller)
{
    if (FAILED(result)) {
        webViewLoaderErrorMessageBox(result);
        return result;
    }

    fController = controller;

    ICoreWebView2Controller2_AddRef(fController);
    ICoreWebView2Controller2_put_IsVisible(fController, false);
    ICoreWebView2Controller2_get_CoreWebView2(fController, &fView);
    ICoreWebView2_add_NavigationCompleted(fView, fHandler, nullptr);
    ICoreWebView2_add_WebMessageReceived(fView, fHandler, nullptr);

    ICoreWebView2Settings2* settings;
    ICoreWebView2_get_Settings(fView, reinterpret_cast<ICoreWebView2Settings**>(&settings));
    ICoreWebView2Settings_put_IsStatusBarEnabled(settings, false);
    
    LPWSTR buf;
    ICoreWebView2Settings2_get_UserAgent(settings, &buf);
    std::wstring userAgent(buf);
    userAgent += L" ";
    userAgent += TO_LPCWSTR(fUserAgent.buffer());
    ICoreWebView2Settings2_put_UserAgent(settings, userAgent.c_str());

    // Run pending requests

    onSize(getWidth(), getHeight());

    // Edge WebView2 currently only supports alpha=0 or alpha=1

    const uint32_t rgba = getBackgroundColor();
    COREWEBVIEW2_COLOR color;
    
    color.A = static_cast<BYTE>(rgba & 0x000000ff);
    color.R = static_cast<BYTE>(rgba >> 24);
    color.G = static_cast<BYTE>((rgba & 0x00ff0000) >> 16);
    color.B = static_cast<BYTE>((rgba & 0x0000ff00) >> 8 );

    ICoreWebView2Controller2_put_DefaultBackgroundColor(
        reinterpret_cast<ICoreWebView2Controller2 *>(fController), color);

    for (StringList::const_iterator it = fInjectedScripts.cbegin(); it != fInjectedScripts.cend(); ++it) {
        ICoreWebView2_AddScriptToExecuteOnDocumentCreated(fView, TO_LPCWSTR(*it), 0);
    }

    if (! fUrl.isEmpty()) {
        ICoreWebView2_Navigate(fView, TO_LPCWSTR(fUrl));
    }

    fReady = true;

    return S_OK;
}

HRESULT EdgeWebView::handleWebView2NavigationCompleted(ICoreWebView2 *sender,
                                                       ICoreWebView2NavigationCompletedEventArgs *eventArgs)
{
    (void)sender;
    (void)eventArgs;

    ICoreWebView2Controller2_put_IsVisible(fController, true);
    handleLoadFinished();
    
    return S_OK;
}

HRESULT EdgeWebView::handleWebView2WebMessageReceived(ICoreWebView2 *sender,
                                                      ICoreWebView2WebMessageReceivedEventArgs *eventArgs)
{
    // Edge WebView2 does not provide access to raw values, resort to parsing JSON.

    LPWSTR jsonStr;
    ICoreWebView2WebMessageReceivedEventArgs_get_WebMessageAsJson(eventArgs, &jsonStr);
    Variant value = Variant::fromJSON(TO_LPCSTR(jsonStr));

    if (value.isArray()) {
        handleScriptMessage(value);
    }

    CoTaskMemFree(jsonStr);

    (void)sender;
    (void)eventArgs;

    return S_OK;
}

void EdgeWebView::errorMessageBox(std::wstring message)
{
    MessageBox(0, message.c_str(), TEXT(DISTRHO_PLUGIN_NAME), MB_OK | MB_ICONEXCLAMATION);
}

void EdgeWebView::webViewLoaderErrorMessageBox(HRESULT result)
{
    std::wstringstream wss;
    wss << "The Microsoft Edge WebView2 Runtime could not be initialized.\n"
        << "Code 0x" << std::hex << result
        << "\n\n"
        << "Make sure it is installed on your system. Clicking OK will open its"
        << " download page. Once installed close and reopen the plugin window."
        << "\n\n";

    std::wstring ws = wss.str();

    const int id = MessageBox(0, ws.c_str(), TEXT(DISTRHO_PLUGIN_NAME),
        MB_OKCANCEL | MB_ICONEXCLAMATION);

    if (id == IDOK) {
        ShellExecute(0, L"open", L"" WEBVIEW2_DOWNLOAD_URL, 0, 0, SW_SHOWNORMAL);
    }
}

LRESULT CALLBACK HelperWindowProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    if (umsg == WM_ERASEBKGND) {
        EdgeWebView* view = reinterpret_cast<EdgeWebView *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        const uint32_t rgba = view->getBackgroundColor();

        if (rgba != 0) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            const COLORREF bgr = ((rgba & 0xff000000) >> 24) | ((rgba & 0x00ff0000) >> 8)
                                | ((rgba & 0x0000ff00) << 8);
            SetBkColor((HDC)wParam, bgr);
            ExtTextOut((HDC)wParam, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
        }

        return 1;
    }

    return DefWindowProc(hwnd, umsg, wParam, lParam);
}

LRESULT CALLBACK KeyboardFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{    
    // HC_ACTION means wParam & lParam contain info about keystroke message
    if (nCode == HC_ACTION) {
        WCHAR className[256];
        HWND hwnd = GetFocus();
        HWND helperHwnd = 0;
        int level = 0;

        // Check if focused window belongs to the hierarchy of one of our plugin instances
        // Max 3 levels is reasonable for reaching plugin window from a Chrome child window
        while (level++ < 3) { 
            GetClassName(hwnd, className, sizeof(className));

            if ((wcswcs(className, L"EdgeWebView") != 0)
                    && (wcswcs(className, WXSTR(DPF_WEBUI_PROJECT_ID_HASH)) != 0)) {
                helperHwnd = hwnd;
                break;
            }

            hwnd = GetParent(hwnd);
        }

        if (helperHwnd != 0) {
            EdgeWebView* view = reinterpret_cast<EdgeWebView *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            KBDLLHOOKSTRUCT* lpData = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
            const bool focus = view->getKeyboardFocus();

            if (view->lowLevelKeyboardHookCallback) {
                view->lowLevelKeyboardHookCallback((UINT)wParam, lpData, focus);
            }

            if (focus) {
                // Let keystroke reach web view
            } else {
                // Do not allow some keystrokes to reach the web view to keep
                // consistent behavior across all platforms, keystrokes should
                // be consumed at a single place. Still need to allow some
                // combinations like Alt-Tab to pass.
                if (((lpData->vkCode >= 'A') && (lpData->vkCode <= 'Z'))
                        || ((lpData->vkCode >= '0') && (lpData->vkCode <= '9'))) {
                    return 1;
                }
            }
        }
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}
