/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#ifndef WEB_UI_BASE_HPP
#define WEB_UI_BASE_HPP

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "distrho/extra/Mutex.hpp"

#include "extra/UIEx.hpp"
#include "extra/StringHash.hpp"
#include "Variant.hpp"

START_NAMESPACE_DISTRHO

const char * const kWebViewUserAgent = "dpfwebui/1.0";

static const uintptr_t kDestinationAll = 0;
static const uintptr_t kDestinationWebView = 1;

static const uintptr_t kExcludeNone = 0;

class WebUIBase : public UIEx
{
public:
    typedef std::function<Variant(const char*)> FunctionArgumentSerializer;

    WebUIBase(uint widthCssPx, uint heightCssPx, float initPixelRatio,
                FunctionArgumentSerializer funcArgSerializer = nullptr);
    virtual ~WebUIBase() {}

    void callback(const char* function, Variant args = Variant::createArray(),
                    uintptr_t destination = kDestinationAll, uintptr_t exclude = kExcludeNone);

protected:
    typedef std::function<void()> UiBlock;
    void queue(const UiBlock& block);

    typedef std::function<void(const Variant& payload, uintptr_t origin)> FunctionHandler;
    const FunctionHandler& getFunctionHandler(const char* name);
    void setFunctionHandler(const char* name, int argCount, const FunctionHandler& handler);

    bool isDryRun();
    
    uint getInitWidthCSS() const { return fInitWidthCssPx; }
    uint getInitHeightCSS() const { return fInitHeightCssPx; }

    void uiIdle() override;

    void parameterChanged(uint32_t index, float value) override;
#if DISTRHO_PLUGIN_WANT_PROGRAMS
    void programLoaded(uint32_t index) override;
#endif
#if DISTRHO_PLUGIN_WANT_STATE
    void stateChanged(const char* key, const char* value) override;
#endif
    void sampleRateChanged(double newSampleRate) override;

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    void sharedMemoryCreated(uint8_t* ptr) override;
#endif

    virtual void postMessage(const Variant& payload, uintptr_t destination, uintptr_t exclude) = 0;
    virtual void onMessageReceived(const Variant& payload, uintptr_t origin);

    void handleMessage(const Variant& payload, uintptr_t origin);

    Variant serializeFunctionArgument(const char* function);

private:
    void setBuiltInFunctionHandlers();

    uint fInitWidthCssPx;
    uint fInitHeightCssPx;
    FunctionArgumentSerializer fFuncArgSerializer;
    Mutex fUiQueueMutex;
    std::queue<UiBlock> fUiQueue;

    typedef std::pair<int, FunctionHandler> ArgumentCountAndFunctionHandler;
    typedef std::unordered_map<String, ArgumentCountAndFunctionHandler> FunctionHandlerMap;
    FunctionHandlerMap fHandler;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebUIBase)

};

END_NAMESPACE_DISTRHO

#endif  // WEB_UI_BASE_HPP
