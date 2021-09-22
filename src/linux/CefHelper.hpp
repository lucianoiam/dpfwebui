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

#include "ipc.h"
#include "ipc_message.h"

class CefHelper : public CefApp, public CefClient, 
                  public CefBrowserProcessHandler, public CefLoadHandler
{
public:
    CefHelper(const ipc_conf_t& conf);
    virtual ~CefHelper();

    void runMainLoop();

    // CefClient

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }
    
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }
  
    // CefBrowserProcessHandler
    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine) override;

    // CefLoadHandler
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

private:
    int  readMessage();
    void dispatch(const tlv_t& packet);
    void realize(const msg_win_cfg_t *config);

    ipc_t*     fIpc;
    bool       fRunMainLoop;
    ::Display* fDisplay;
    ::Window   fContainer;
    
    CefRefPtr<CefBrowser> fBrowser;
    std::string           fInjectedScript;

    // Include the CEF default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelper);
};

class CefHelperSubprocess : public CefApp, public CefClient,
                            public CefRenderProcessHandler, public CefV8Handler
{
public:
    CefHelperSubprocess();
    virtual ~CefHelperSubprocess();

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
    ipc_t*    fIpc;
    CefString fInjectedScript;

    IMPLEMENT_REFCOUNTING(CefHelperSubprocess);
};

#endif // CEF_HELPER_HPP
