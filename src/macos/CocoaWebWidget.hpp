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

#ifndef COCOAWEBWIDGET_HPP
#define COCOAWEBWIDGET_HPP

#include "AbstractWebWidget.hpp"

// While it is possible to #import Obj-C headers here, that would force all
// source files importing CocoaWebWidget.hpp to do so before any other project
// headers to avoid symbol name collisions. Do not make any assumption.

START_NAMESPACE_DISTRHO

class CocoaWebWidget : public AbstractWebWidget
{
public:
    CocoaWebWidget(Widget *parentWidget);
    ~CocoaWebWidget();

    void setBackgroundColor(uint32_t rgba) override;
    void navigate(String& url) override;
    void runScript(String& source) override;
    void injectScript(String& source) override;
    
    // Allow calling some protected methods from Objective-C instances
    
    void didFinishNavigation() { handleLoadFinished(); }
    void didReceiveScriptMessage(const ScriptValueVector& args) { handleScriptMessage(args); }

protected:
    void onResize(const ResizeEvent& ev) override;
    void onPositionChanged(const PositionChangedEvent& ev) override;
    bool onKeyboard(const KeyboardEvent& ev) override;

private:
    void updateWebViewFrame();

    void *fView;
    void *fDelegate;

    uint fLastKeyboardEventTime;

};

END_NAMESPACE_DISTRHO

#endif  // COCOAWEBWIDGET_HPP
