/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>

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
