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

#ifndef IPC_CHANNEL_HPP
#define IPC_CHANNEL_HPP

#include "distrho/extra/LeakDetector.hpp"
#include "distrho/extra/String.hpp"

#include "ipc.h"
#include "ipc_message.h"

START_NAMESPACE_DISTRHO

class IpcChannel
{
public:
    IpcChannel(int fdr, int fdw);
    virtual ~IpcChannel();

    int getFdRead() const;
    int getFdWrite() const;

    int read(tlv_t* packet, int timeoutMs = 0) const;

    int write(msg_opcode_t opcode) const;
    int write(msg_opcode_t opcode, String& str) const;
    int write(msg_opcode_t opcode, const void* payload, int payloadSize) const; 

private:
    int waitAndRead(tlv_t* packet) const;

    ipc_t* fIpc;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IpcChannel)

};

END_NAMESPACE_DISTRHO

#endif  // IPC_CHANNEL_HPP
