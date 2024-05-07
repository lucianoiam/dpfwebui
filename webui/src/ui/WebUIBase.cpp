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

#include "WebUIBase.hpp"
#include "DistrhoPluginInfo.h"

#include "distrho/DistrhoPluginUtils.hpp"
#include "distrho/extra/Base64.hpp"

USE_NAMESPACE_DISTRHO

WebUIBase::WebUIBase(uint widthCssPx, uint heightCssPx, float initPixelRatio,
                        FunctionArgumentSerializer funcArgSerializer)
    : UIEx(initPixelRatio * widthCssPx, initPixelRatio * heightCssPx)
    , fInitWidthCssPx(widthCssPx)
    , fInitHeightCssPx(heightCssPx)
    , fFuncArgSerializer(funcArgSerializer != nullptr ? funcArgSerializer
                            : [](const char* f) { return f; })
{
    setBuiltInFunctionHandlers();
}

void WebUIBase::callback(const char* function, Variant args, uintptr_t destination, uintptr_t exclude)
{
    args.insertArrayItem(0, serializeFunctionArgument(function));
    postMessage(args, destination, exclude);
}

void WebUIBase::queue(const UiBlock& block)
{
    fUiQueueMutex.lock();
    fUiQueue.push(block);
    fUiQueueMutex.unlock();
}

const WebUIBase::FunctionHandler& WebUIBase::getFunctionHandler(const char* name)
{
    return fHandler[serializeFunctionArgument(name).asString()].second;
}

void WebUIBase::setFunctionHandler(const char* name, int argCount, const FunctionHandler& handler)
{
    fHandler[serializeFunctionArgument(name).asString()] = std::make_pair(argCount, handler);
}

bool WebUIBase::isDryRun()
{
    // When running as a plugin the UI ctor/dtor can be repeatedly called with
    // no parent window available, avoid allocating resources in such cases.
    return ! isStandalone() && (getParentWindowHandle() == 0);
}

void WebUIBase::uiIdle()
{
    UIEx::uiIdle();
    fUiQueueMutex.lock();

    while (! fUiQueue.empty()) {
        const UiBlock block = fUiQueue.front();
        fUiQueue.pop();
        block();
    }

    fUiQueueMutex.unlock();
}

void WebUIBase::parameterChanged(uint32_t index, float value)
{
    callback("parameterChanged", { index, value });
}

#if DISTRHO_PLUGIN_WANT_PROGRAMS
void WebUIBase::programLoaded(uint32_t index)
{
    callback("programLoaded", { index });
}
#endif

#if DISTRHO_PLUGIN_WANT_STATE
void WebUIBase::stateChanged(const char* key, const char* value)
{
    callback("stateChanged", { key, value });
}
#endif

void WebUIBase::sampleRateChanged(double newSampleRate)
{
    callback("sampleRateChanged", { newSampleRate });
}

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
void WebUIBase::sharedMemoryCreated(uint8_t*)
{
    callback("sharedMemoryCreated");
}
#endif

void WebUIBase::onMessageReceived(const Variant& payload, uintptr_t origin)
{
    (void)payload;
    (void)origin;
}

void WebUIBase::handleMessage(const Variant& payload, uintptr_t origin)
{
    if (! payload.isArray() || (payload.getArraySize() == 0)) {
        d_stderr2("Message must be a non-empty array");
        return;
    }

    String function = payload[0].asString();

    if (fHandler.find(function) == fHandler.end()) {
        d_stderr2("Unknown WebUI function");
        return;
    }

    const Variant handlerArgs = payload.sliceArray(1);
    
    ArgumentCountAndFunctionHandler handler = fHandler[function];
    const int argsCount = handlerArgs.getArraySize();

    if (argsCount < handler.first) {
        d_stderr2("Missing WebUI function arguments (%d < %d)", argsCount, handler.first);
        return;
    }

    handler.second(handlerArgs, origin);
}

Variant WebUIBase::serializeFunctionArgument(const char* function)
{
    return fFuncArgSerializer(function);
}

void WebUIBase::setBuiltInFunctionHandlers()
{
    setFunctionHandler("getInitWidthCSS", 0, [this](const Variant&, uintptr_t origin) {
        callback("getInitWidthCSS", { static_cast<double>(getInitWidthCSS()) }, origin);
    });

    setFunctionHandler("getInitHeightCSS", 0, [this](const Variant&, uintptr_t origin) {
        callback("getInitHeightCSS", { static_cast<double>(getInitHeightCSS()) }, origin);
    });

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    setFunctionHandler("sendNote", 3, [this](const Variant& args, uintptr_t) {
        sendNote(
            static_cast<uint8_t>(args[0].getNumber()),  // channel
            static_cast<uint8_t>(args[1].getNumber()),  // note
            static_cast<uint8_t>(args[2].getNumber())   // velocity
        );
    });
#endif

    setFunctionHandler("getSampleRate", 0, [this](const Variant&, uintptr_t origin) {
        callback("getSampleRate", { getSampleRate() }, origin);
    });

    setFunctionHandler("editParameter", 2, [this](const Variant& args, uintptr_t) {
        editParameter(
            static_cast<uint32_t>(args[0].getNumber()), // index
            static_cast<bool>(args[1].getBoolean())     // started
        );
    });

    setFunctionHandler("setParameterValue", 2, [this](const Variant& args, uintptr_t) {
        setParameterValue(
            static_cast<uint32_t>(args[0].getNumber()), // index
            static_cast<float>(args[1].getNumber())     // value
        );
    });

#if DISTRHO_PLUGIN_WANT_STATE
    setFunctionHandler("setState", 2, [this](const Variant& args, uintptr_t) {
        setState(
            args[0].getString(), // key
            args[1].getString()  // value
        );
    });
#endif

#if DISTRHO_PLUGIN_WANT_STATE && defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    setFunctionHandler("writeSharedMemory", 2, [this](const Variant& args, uintptr_t) {
# if DPF_WEBUI_PROTOCOL_BINARY
        BinaryData data = args[0].getBinaryData();
# else
        std::vector<uint8_t> data = d_getChunkFromBase64String(args[0].getString());
# endif
        writeSharedMemory(
            data.data(),                             // data
            static_cast<size_t>(data.size()),        // size
            static_cast<size_t>(args[1].getNumber()) // offset
        );
    });
#endif // DISTRHO_PLUGIN_WANT_STATE && DPF_WEBUI_SHARED_MEMORY_SIZE

    setFunctionHandler("isStandalone", 0, [this](const Variant&, uintptr_t origin) {
        callback("isStandalone", { isStandalone() }, origin);
    });
}
