// Based on https://github.com/jchv/webview2-in-mingw
// lucianoiam - more event types and __EVENT__ template
#pragma once
#define CINTERFACE
#include <functional>

#include "WebView2.h"

class EventHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                     public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                     public ICoreWebView2ContentLoadingEventHandler,
                     public ICoreWebView2NavigationCompletedEventHandler
                  // public ICoreWebView2__EVENT__Handler
{
public:
    EventHandler();
    virtual ~EventHandler();

    EventHandler(const EventHandler&) = delete;
    EventHandler(EventHandler&&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;
    EventHandler& operator=(EventHandler&&) = delete;

    std::function<HRESULT(HRESULT result, ICoreWebView2Environment* created_environment)> EnvironmentCompleted;
    std::function<HRESULT(HRESULT result, ICoreWebView2Controller* controller)> ControllerCompleted;
    std::function<HRESULT(ICoreWebView2 *sender, ICoreWebView2ContentLoadingEventArgs *args)> ContentLoading;
    std::function<HRESULT(ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)> NavigationCompleted;
 // std::function<HRESULT(HRESULT result, ... see WebView2.h for arguments ... )> __EVENT__;
};
