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

#ifndef SCRIPTVALUE_HPP
#define SCRIPTVALUE_HPP

#include "extra/String.hpp"

#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

class ScriptValue {
public:
    ScriptValue()         : fNull(true) {};
    ScriptValue(bool b)   : fNull(false), fB(b) {};
    ScriptValue(double d) : fNull(false), fD(d) {};
    ScriptValue(String s) : fNull(false), fS(s) {};

    bool   isNull()   const { return fNull; }
    bool   asBool()   const { return fB; }
    double asDouble() const { return fD; }
    String asString() const { return fS; }

    operator bool()   const { return fB; }
    operator double() const { return fD; }
    operator String() const { return fS; }

private:
    bool   fNull;
    bool   fB;
    double fD;
    String fS;

};

END_NAMESPACE_DISTRHO

#endif // SCRIPTVALUE_HPP
