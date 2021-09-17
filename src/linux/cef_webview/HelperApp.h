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

#ifndef HELPER_APP_HPP
#define HELPER_APP_HPP

#include "include/cef_app.h"

#include "linux/ipc.h"

// Implement application-level callbacks for the browser process.
class HelperApp : public CefApp, public CefBrowserProcessHandler
{
public:
    HelperApp(ipc_t* ipc);

    void run();

    // CefApp methods

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }

    // CefBrowserProcessHandler methods

    virtual void OnContextInitialized() override;

private:
    void dispatch(const tlv_t* packet);

    bool   fRun;
    ipc_t* fIpc;
    
    // Include the default reference counting implementation
    IMPLEMENT_REFCOUNTING(HelperApp);
};

#endif  // HELPER_APP_HPP
