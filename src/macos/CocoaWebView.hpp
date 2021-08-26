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

#ifndef COCOA_WEBVIEW_HPP
#define COCOA_WEBVIEW_HPP

#include "AbstractWebView.hpp"

// While it is possible to #import Obj-C headers here, that would force all
// source files importing CocoaWebView.hpp to do so before any other project
// headers to avoid symbol name collisions. Do not make any assumption.

START_NAMESPACE_DISTRHO

class CocoaWebView : public AbstractWebView
{
public:
    CocoaWebView(uintptr_t parentWindowHandle);
    ~CocoaWebView();

    void setBackgroundColor(uint32_t rgba) override;
    void setSize(uint width, uint height) override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;

    // Allow calling some protected methods from Objective-C instances
    
    void didFinishNavigation() { handleLoadFinished(); }
    void didReceiveScriptMessage(const JsValueVector& args) { handleScriptMessage(args); }

private:
    void *fView;
    void *fDelegate;

    //uint fLastKeyboardEventTime;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CocoaWebView)

};

END_NAMESPACE_DISTRHO

#endif  // COCOA_WEBVIEW_HPP
