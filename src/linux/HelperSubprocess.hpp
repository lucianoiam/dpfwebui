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

#ifndef HELPERSUBPROCESS_HPP
#define HELPERSUBPROCESS_HPP

#include <cstdint>
#include <sys/types.h>

#include "extra/String.hpp"
#include "extra/Thread.hpp"

#include "ipc.h"

START_NAMESPACE_DISTRHO

class HelperIpcReader : public Thread
{
public:
    HelperIpcReader() : Thread("ipc_reader") {}

    void setIpc(ipc_t *ipc) { fIpc = ipc; };

    virtual void run() override;

    // TODO: callback function

private:
    ipc_t* fIpc;

};

class HelperSubprocess
{
public:
    HelperSubprocess();
    ~HelperSubprocess();

    bool isRunning() { return fPid != 0; }
    int  spawn();
    void terminate();

    int navigate(String url);
    int reparent(uintptr_t windowId);

private:
    int send(char opcode, const void *data, int size);

    int             fPipeFd[2][2];
    pid_t           fPid;
    ipc_t*          fIpc;
    HelperIpcReader fIpcReader;

};

END_NAMESPACE_DISTRHO

#endif  // HELPERSUBPROCESS_HPP
