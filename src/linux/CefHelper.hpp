/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CEF_HELPER_HPP
#define CEF_HELPER_HPP

#include <vector>

#include <X11/Xlib.h>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"

#include "IpcChannel.hpp"

class CefHelper : public CefApp, public CefClient, 
                  public CefBrowserProcessHandler, public CefLoadHandler
{
public:
    CefHelper();
    virtual ~CefHelper();

    int run(const CefMainArgs& args);

    // CefClient

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }
    
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }
    
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId sourceProcess,
                                          CefRefPtr<CefProcessMessage> message) override;

    // CefBrowserProcessHandler

    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine) override;

    // CefLoadHandler
    
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             TransitionType transitionType) override;

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

private:
    void runMainLoop();
    void dispatch(const tlv_t& packet);
    void realize(const msg_win_cfg_t* config);
    void navigate(const char* url);
    void runScript(const char* js);
    void injectScript(const char* js);
    void setSize(const msg_win_size_t* size);
    void setKeyboardFocus(bool keyboardFocus);

    float getX11ScaleFactor();

    float       fScaleFactor;
    bool        fRunMainLoop;
    IpcChannel* fIpc;
    ::Display*  fDisplay;
    ::Window    fContainer;
    
    CefRefPtr<CefBrowser>   fBrowser;
    CefRefPtr<CefListValue> fInjectedScripts;

    // Include the CEF default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelper);
};

class CefSubprocess : public CefApp, public CefClient,
                      public CefRenderProcessHandler, public CefV8Handler
{
public:
    CefSubprocess();
    virtual ~CefSubprocess() {}

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }

    // CefClient
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId sourceProcess,
                                          CefRefPtr<CefProcessMessage> message) override;

    // CefRenderProcessHandler
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    // CefV8Handler
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
    CefRefPtr<CefBrowser>   fBrowser;
    CefRefPtr<CefListValue> fInjectedScripts;

    IMPLEMENT_REFCOUNTING(CefSubprocess);
};

#endif // CEF_HELPER_HPP
