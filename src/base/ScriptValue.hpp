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

#include <ostream>

#include "extra/String.hpp"

#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

class ScriptValue {
public:
    enum Type {
        TNull,
        TBool,
        TDouble,
        TString
    };

    ScriptValue()         : fT(TNull),   fB(false), fD(0)  {};
    ScriptValue(bool b)   : fT(TBool),   fB(b),     fD(0)  {};
    ScriptValue(double d) : fT(TDouble), fB(false), fD(d)  {};
    ScriptValue(String s) : fT(TString), fB(false), fD(0), fS(s) {};

    bool   isNull()    const { return fT == TNull; }
    bool   getBool()   const { return fB; }
    double getDouble() const { return fD; }
    String getString() const { return fS; }

    operator bool()   const { return fB; }
    operator double() const { return fD; }
    operator String() const { return fS; }

    std::ostream& operator<<(std::ostream &os) {
        switch (fT) {
            case TNull:
                os << "null";
                break;
            case TBool:
                os << (fB ? "true" : "false");
                break;
            case TDouble:
                os << fD;
                break;
            case TString: {
                const char *buf = fS.buffer();
                int len = fS.length();
                for (int i = 0; i < len; i++) {
                    if (buf[i] != '"') {
                        os << buf[i];
                    } else {
                        os << "\\\"";
                    }
                }
                break;
            }
            default:
                break;
        }
        return os; 
    }

private:
    Type   fT;
    bool   fB;
    double fD;
    String fS;

};

END_NAMESPACE_DISTRHO

#endif // SCRIPTVALUE_HPP
