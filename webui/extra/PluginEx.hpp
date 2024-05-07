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

#ifndef PLUGIN_EX_HPP
#define PLUGIN_EX_HPP

#include <map>

#include "DistrhoPlugin.hpp"

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
# if ! DISTRHO_PLUGIN_WANT_STATE
#  error Shared memory support requires DISTRHO_PLUGIN_WANT_STATE
# endif
# include "extra/SharedMemory.hpp"
#endif 

START_NAMESPACE_DISTRHO

// This class adds some goodies to DISTRHO::Plugin like shared memory support

class PluginEx : public Plugin
{
public:
    PluginEx(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount);
    virtual ~PluginEx() {}

#if DISTRHO_PLUGIN_WANT_STATE
    void   initState(uint32_t index, State& state) override;
    void   setState(const char* key, const char* value) override;
# if DISTRHO_PLUGIN_WANT_FULL_STATE
    String getState(const char* key) const override;
# endif
#endif

    // TODO : Move shared memory ownership back to Plugin once the
    //        Plugin::updateStateValue() call works for all formats
    // https://github.com/DISTRHO/DPF/issues/410#issuecomment-1414435206
    // https://github.com/DISTRHO/OneKnob-Series/issues/6

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    uint8_t* getSharedMemoryPointer() const noexcept;
    bool     writeSharedMemory(const uint8_t* data, size_t size, size_t offset = 0) const noexcept;
#endif

protected:
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    virtual void sharedMemoryWillDisconnect() {}
    virtual void sharedMemoryConnected(uint8_t* ptr)
    {
        (void)ptr;
    }

    virtual void sharedMemoryWritten(uint8_t* data, size_t size, size_t offset)
    {
        (void)data;
        (void)size;
        (void)offset;
    }
#endif

private:
#if defined(DPF_WEBUI_NETWORK_UI)
    uint32_t fStateIndexWsPort;
#endif
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    uint32_t fStateIndexShMemFile;
    uint32_t fStateIndexShMemData;
    SharedMemory<uint8_t,DPF_WEBUI_SHARED_MEMORY_SIZE> fMemory;
#endif
#if DPF_WEBUI_ZEROCONF
    uint32_t fStateIndexZeroconfPublish;
    uint32_t fStateIndexZeroconfId;
    uint32_t fStateIndexZeroconfName;
#endif
#if DISTRHO_PLUGIN_WANT_FULL_STATE
    typedef std::map<String,String> StateMap;
    StateMap fState;
#endif
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEx)

};

END_NAMESPACE_DISTRHO

#endif  // PLUGIN_EX_HPP
