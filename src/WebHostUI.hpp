/*
 * Apices - Audio Plugins In C++ & ES6
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef WEBHOSTUI_HPP
#define WEBHOSTUI_HPP

#include "DistrhoUI.hpp"

#include <vector>

#include "Platform.hpp"

START_NAMESPACE_DISTRHO

class WebHostUI : public UI, private WebWidgetEventHandler
{
public:
    WebHostUI(uint baseWidth = 0, uint baseHeight = 0, uint32_t backgroundColor = 0xffffffff);
    virtual ~WebHostUI() {};

protected:
    void onDisplay() override;
    void uiReshape(uint width, uint height) override;

    void parameterChanged(uint32_t index, float value) override;
#if (DISTRHO_PLUGIN_WANT_STATE == 1)
    void stateChanged(const char* key, const char* value) override;
#endif
    
    uint getInitWidth() const;
    uint getInitHeight() const;

    platform::WebWidget& webWidget() { return fWebWidget; }

    void webPostMessage(const ScriptValueVector& args);

    void flushInitMessageQueue();
    void setKeyboardFocus(bool focus);

    virtual void webContentReady() {};
    virtual void webMessageReceived(const ScriptValueVector& args) { (void)args; };

private:
    // WebWidgetEventHandler

    virtual void handleWebWidgetContentLoadFinished() override;
    virtual void handleWebWidgetScriptMessageReceived(const ScriptValueVector& args) override;

    typedef std::vector<ScriptValueVector> InitMessageQueue;
    
    platform::WebWidget fWebWidget;
    InitMessageQueue    fInitMsgQueue;
    bool                fFlushedInitMsgQueue;
    uint32_t            fBackgroundColor;
    uint                fInitWidth;
    uint                fInitHeight;

};

END_NAMESPACE_DISTRHO

#endif  // WEBHOSTUI_HPP
