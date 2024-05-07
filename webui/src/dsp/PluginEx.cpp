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

#include <cstdlib>

#include "extra/PluginEx.hpp"

// This is ugly but __COUNTER__ alone cannot solve the problem
#if defined(DPF_WEBUI_NETWORK_UI)
# define COUNT_0 1
#else
# define COUNT_0 0
#endif
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE) // DistrhoPluginInfo.h
# define COUNT_1 2
#else
# define COUNT_1 0
#endif
#if DPF_WEBUI_ZEROCONF // DistrhoPluginInfo.h
# define COUNT_2 3
#else
# define COUNT_2 0
#endif

#define INTERNAL_STATE_COUNT (COUNT_0 + COUNT_1 + COUNT_2)

#if DPF_WEBUI_ZEROCONF
#include <random>

std::string gen_uuid() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return res;
}
#endif

PluginEx::PluginEx(uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
    : Plugin(parameterCount, programCount, stateCount + INTERNAL_STATE_COUNT)
    // stateCount is the last user state index
#if defined(DPF_WEBUI_NETWORK_UI)
    , fStateIndexWsPort(stateCount + __COUNTER__)
#endif
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    , fStateIndexShMemFile(stateCount + __COUNTER__)
    , fStateIndexShMemData(stateCount + __COUNTER__)
#endif
#if DPF_WEBUI_ZEROCONF
    , fStateIndexZeroconfPublish(stateCount + __COUNTER__)
    , fStateIndexZeroconfId(stateCount + __COUNTER__)
    , fStateIndexZeroconfName(stateCount + __COUNTER__)
#endif
{}

#if DISTRHO_PLUGIN_WANT_STATE
void PluginEx::initState(uint32_t index, State& state)
{
    (void)index;
    (void)state;
# if defined(DPF_WEBUI_NETWORK_UI)
    if (index == fStateIndexWsPort) {
        state.key = "_ws_port";
        state.defaultValue = "-1";
    }
# endif
# if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    if (index == fStateIndexShMemFile) {
        state.key = "_shmem_file";
    } else if (index == fStateIndexShMemData) {
        state.key = "_shmem_data";
    }
# endif
# if DPF_WEBUI_ZEROCONF
    if (index == fStateIndexZeroconfPublish) {
        state.key = "_zc_publish";
        state.defaultValue = "true";
    } else if (index == fStateIndexZeroconfId) {
        state.key = "_zc_id";
        state.defaultValue = gen_uuid().c_str();
    } else if (index == fStateIndexZeroconfName) {
        state.key = "_zc_name";
        state.defaultValue = DISTRHO_PLUGIN_NAME;
    }
# endif
# if DISTRHO_PLUGIN_WANT_FULL_STATE
    fState[state.key] = state.defaultValue;
# endif
}

void PluginEx::setState(const char* key, const char* value)
{
    (void)key;
    (void)value;
# if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    // Do not persist _shmem_* values by returning before setting fState
    if (std::strcmp("_shmem_file", key) == 0) {
        if (value[0] != '\0') {
            if (std::strcmp(value, "close") == 0) {
                sharedMemoryWillDisconnect();
                fMemory.close();
            } else {
                uint8_t* ptr = fMemory.connect(value);
                if (ptr != nullptr) { 
                    sharedMemoryConnected(ptr);
                } else {
                    d_stderr2("PluginEx : could not connect to shared memory");
                }
            }
        }
        return;
    } else if (std::strcmp(key, "_shmem_data") == 0) {
        char* v2;
        size_t size = std::strtol(value, &v2, 10);
        size_t offset = std::strtol(v2 + 1, nullptr, 10);
        sharedMemoryWritten(getSharedMemoryPointer(), size, offset);
        return;
    }
# endif
# if DISTRHO_PLUGIN_WANT_FULL_STATE
    fState[String(key)] = value;
# endif
}

# if DISTRHO_PLUGIN_WANT_FULL_STATE
String PluginEx::getState(const char* key) const
{
    StateMap::const_iterator it = fState.find(String(key));

    if (it == fState.end()) {
        return String();
    }
    
    return it->second;
}
# endif

#endif // DISTRHO_PLUGIN_WANT_STATE

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
uint8_t* PluginEx::getSharedMemoryPointer() const noexcept
{
    return fMemory.getDataPointer();
}

bool PluginEx::writeSharedMemory(const uint8_t* data, size_t size, size_t offset) const noexcept
{
    uint8_t* ptr = fMemory.getDataPointer();

    if (ptr == nullptr) {
        return false;
    }

    std::memcpy(ptr + offset, data, size);

    return true;
}
#endif
