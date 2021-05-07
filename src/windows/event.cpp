#define CINTERFACE
#include "event.h"
#include "WebView2.h"

template <typename T>
static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject)
{
	return E_NOINTERFACE;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_AddRef(T* This)
{
	return 1;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_Release(T* This)
{
	return 1;
}

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

EventHandler::EventHandler() :
	ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler{&EventInterfaces_EnvironmentCompletedHandlerVtbl},
	ICoreWebView2CreateCoreWebView2ControllerCompletedHandler{&EventInterfaces_ControllerCompletedHandlerVtbl}
{
}

EventHandler::~EventHandler() = default;
