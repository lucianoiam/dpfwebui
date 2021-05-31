// Based on https://github.com/jchv/webview2-in-mingw
// lucianoiam - more event types and template
#define CINTERFACE
#include "event.h"
#include "WebView2.h"

template <typename T>
static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject)
{
    (void)This;
    (void)riid;
    (void)ppvObject;
    return E_NOINTERFACE;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_AddRef(T* This)
{
    (void)This;
    return 1;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_Release(T* This)
{
    (void)This;
    return 1;
}


// ---- EnvironmentCompleted ----

static HRESULT STDMETHODCALLTYPE EventInterfaces_EnvironmentCompleted_Invoke(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT result,
    ICoreWebView2Environment* created_environment)
{
    return static_cast<EventHandler*>(This)->EnvironmentCompleted(result, created_environment);
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl EventInterfaces_EnvironmentCompletedHandlerVtbl =
{
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces_EnvironmentCompleted_Invoke,
};


// ---- ControllerCompleted ----

static HRESULT STDMETHODCALLTYPE EventInterfaces_ControllerCompleted_Invoke(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, HRESULT result,
    ICoreWebView2Controller* controller)
{
    return static_cast<EventHandler*>(This)->ControllerCompleted(result, controller);
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl EventInterfaces_ControllerCompletedHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces_ControllerCompleted_Invoke,
};


// ---- NavigationCompletedEvent ----

static HRESULT STDMETHODCALLTYPE EventInterfacesNavigationCompletedEvent_Invoke(
    ICoreWebView2NavigationCompletedEventHandler* This,
    ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)
{
    return static_cast<EventHandler*>(This)->NavigationCompleted(sender, args);
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl EventInterfacesNavigationCompletedEventHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfacesNavigationCompletedEvent_Invoke,
};


// ---- __EVENT__ ----

/*static HRESULT STDMETHODCALLTYPE EventInterfaces__EVENT__Event_Invoke(
    ICoreWebView2__EVENT__EventHandler* This, HRESULT result,
    ICoreWebView2Controller* controller)
{
    return static_cast<EventHandler*>(This)->__EVENT__Event(result, controller);
}

static ICoreWebView2__EVENT__EventHandlerVtbl EventInterfaces__EVENT__EventHandlerVtbl = {
    Null_QueryInterface,
    Null_AddRef,
    Null_Release,
    EventInterfaces__EVENT__Event_Invoke,
};*/


EventHandler::EventHandler() :
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&EventInterfaces_EnvironmentCompletedHandlerVtbl},
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&EventInterfaces_ControllerCompletedHandlerVtbl},
    ICoreWebView2NavigationCompletedEventHandler{&EventInterfacesNavigationCompletedEventHandlerVtbl}
 // ICoreWebView2__EVENT__Handler{&EventInterfaces__EVENT__HandlerVtbl}
{
}

EventHandler::~EventHandler() = default;
