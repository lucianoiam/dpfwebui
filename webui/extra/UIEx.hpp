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

#ifndef UI_EX_HPP
#define UI_EX_HPP

#include "DistrhoUI.hpp"

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
# if ! DISTRHO_PLUGIN_WANT_STATE
#  error Shared memory support requires DISTRHO_PLUGIN_WANT_STATE
# endif
# include "extra/SharedMemory.hpp"
#endif 

START_NAMESPACE_DISTRHO

// This class adds some goodies to DISTRHO::UI like shared memory support

class UIEx : public UI
{
public:
    UIEx(uint width = 0, uint height = 0);
    virtual ~UIEx();

    // TODO : Move shared memory ownership back to Plugin once the
    //        Plugin::updateStateValue() call works for all formats
    // https://github.com/DISTRHO/DPF/issues/410#issuecomment-1414435206
    // https://github.com/DISTRHO/OneKnob-Series/issues/6

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    uint8_t* getSharedMemoryPointer() const noexcept;
    bool     writeSharedMemory(const uint8_t* data, size_t size, size_t offset = 0) noexcept;
    void     notifySharedMemoryWillDisconnect();
#endif

protected:
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    void uiIdle() override;

    virtual void sharedMemoryCreated(uint8_t* ptr)
    {
        (void)ptr;
    }
#endif

private:
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    SharedMemory<uint8_t,DPF_WEBUI_SHARED_MEMORY_SIZE> fMemory;
#endif

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIEx)

};

END_NAMESPACE_DISTRHO

#endif  // UI_EX_HPP
