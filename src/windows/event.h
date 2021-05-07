#pragma once
#define CINTERFACE
#include <functional>

#include "WebView2.h"

class EventHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                     public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
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
};
