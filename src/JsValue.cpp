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

#include "JsValue.hpp"

USE_NAMESPACE_DISTRHO

std::ostream& operator<<(std::ostream &os, const JsValue &val) {
    switch (val.getType()) {
        case JsValue::TNull:
            os << "null";
            break;

        case JsValue::TBool:
            os << (val.getBool() ? "true" : "false");
            break;

        case JsValue::TDouble: {
            double d = val.getDouble();
            if (std::isnan(d)) {
                os << "NaN";
            } else if (std::isinf(d)) {
                os << (d < 0 ? "-Inf" : "Inf");
            } else {
                os << d;
            }
            break;
        }

        case JsValue::TString: {
            const String& s = val.getString();
            const char *buf = s.buffer();
            int len = s.length();
            os << '"';
            for (int i = 0; i < len; i++) {
                if (buf[i] != '"') {
                    os << buf[i];
                } else {
                    os << "\\\"";
                }
            }
            os << '"';
            break;
        }

        default:
            break;
    }

    return os;
}
