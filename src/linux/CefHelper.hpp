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

class CefHelper : public CefApp,
                  public CefClient, 
                  public CefBrowserProcessHandler, 
                  public CefRenderProcessHandler,
                  public CefLoadHandler,
                  public CefV8Handler
{
public:
    CefHelper();
    virtual ~CefHelper();

    void createIpc(const ipc_conf_t& conf);

    void runMainLoop();

    // CefClient

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
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

    // CefRenderProcessHandler
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    // CefLoadHandler
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

    // CefV8Handler
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
    void realize(const msg_win_cfg_t *config);
    void dispatch(const tlv_t* packet);

    ipc_t*     fIpc;
    bool       fbRunMainLoop;
    ::Display* fbDisplay;
    ::Window   fbContainer;
    
    CefRefPtr<CefBrowser> fbBrowser;
    std::string           fbInjectedScript;
    CefString             frInjectedScript;

    // Include the CEF default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelper);
};

#endif // CEF_HELPER_HPP
