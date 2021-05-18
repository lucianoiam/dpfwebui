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

#ifndef HELPERIPC_HPP
#define HELPERIPC_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include "extra/Thread.hpp"

START_NAMESPACE_DISTRHO

class HelperIpc : public Thread
{
public:
    HelperIpc();
    virtual ~HelperIpc() override;

    // TO DO: receive callback

    void sendString(int opcode, const String &s);

protected:
    virtual void run() override;

private:
    void send(int opcode, const void *payload, int size);

    int fReadPipe;
    int fWritePipe;

};

END_NAMESPACE_DISTRHO

#endif  // HELPERIPC_HPP
