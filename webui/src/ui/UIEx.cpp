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

#include <cstring>

#include "extra/UIEx.hpp"

UIEx::UIEx(uint width, uint height)
    : UI(width, height)
{}

UIEx::~UIEx()
{
#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
    notifySharedMemoryWillDisconnect();
#endif
}

#if defined(DPF_WEBUI_SHARED_MEMORY_SIZE)
uint8_t* UIEx::getSharedMemoryPointer() const noexcept
{
    return fMemory.getDataPointer();
}

bool UIEx::writeSharedMemory(const uint8_t* data, size_t size, size_t offset) noexcept
{
    uint8_t* ptr = fMemory.getDataPointer();

    if (ptr == nullptr) {
        return false;
    }

    std::memcpy(ptr + offset, data, size);

    String metadata = String(size) + String(';') + String(offset);
    setState("_shmem_data", metadata.buffer());

    return true;
}

void UIEx::notifySharedMemoryWillDisconnect()
{
    setState("_shmem_file", "close"); // PluginEx::sharedMemoryWillDisconnect()
}

void UIEx::uiIdle()
{
    // setState() fails for VST3 when called from constructor
    if (! fMemory.isCreatedOrConnected() && fMemory.create()) {
        setState("_shmem_file", fMemory.getDataFilename());
        sharedMemoryCreated(fMemory.getDataPointer());
    }
}
#endif // DPF_WEBUI_SHARED_MEMORY_SIZE
