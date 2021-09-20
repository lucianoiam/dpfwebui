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

    // CefBrowserProcessHandler

    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> commandLine);

    // CefRenderProcessHandler

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    // CefLoadHandler

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             TransitionType transitionType) override;

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

    // CefV8Handler

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
    void dispatch(const tlv_t* packet);
    void realize(const msg_win_cfg_t *config);

    bool       fRunMainLoop;
    ipc_t*     fIpc;
    ::Display* fDisplay;
    ::Window   fContainer;
    
    std::vector<CefString> fInjectedScripts;
    CefRefPtr<CefBrowser>  fBrowser;

    // Include the default reference counting implementation
    IMPLEMENT_REFCOUNTING(CefHelper);
};

#endif // CEF_HELPER_HPP
