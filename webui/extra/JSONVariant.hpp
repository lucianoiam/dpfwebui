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

#ifndef JSON_VARIANT_HPP
#define JSON_VARIANT_HPP

#include <initializer_list>
#include <utility>

#include "distrho/extra/String.hpp"
#include "thirdparty/cJSON.h"

#include "VariantUtil.hpp"

START_NAMESPACE_DISTRHO

class JSONVariant
{
public:
    JSONVariant() noexcept;
    JSONVariant(bool b) noexcept;
    JSONVariant(double d) noexcept;
    JSONVariant(String s) noexcept;
    JSONVariant(const BinaryData& data) noexcept;

    // Convenience constructors for plugin code
    JSONVariant(int32_t i) noexcept;
    JSONVariant(uint32_t i) noexcept;
    JSONVariant(float f) noexcept;
    JSONVariant(const char* s) noexcept;

    typedef std::pair<const char*,JSONVariant> KeyValue;
    JSONVariant(std::initializer_list<KeyValue> items) noexcept;
    JSONVariant(std::initializer_list<JSONVariant> items) noexcept;

    ~JSONVariant();

    JSONVariant(const JSONVariant& var) noexcept;
    JSONVariant& operator=(const JSONVariant& var) noexcept;
    JSONVariant(JSONVariant&& var) noexcept;
    JSONVariant& operator=(JSONVariant&& var) noexcept;

    static JSONVariant createObject(std::initializer_list<KeyValue> items = {}) noexcept;
    static JSONVariant createArray(std::initializer_list<JSONVariant> items = {}) noexcept;

    bool isNull() const noexcept;
    bool isBoolean() const noexcept;
    bool isNumber() const noexcept;
    bool isString() const noexcept;
    bool isBinaryData() const noexcept;
    bool isArray() const noexcept;
    bool isObject() const noexcept;

    String      asString() const noexcept;
    bool        getBoolean() const noexcept;
    double      getNumber() const noexcept;
    String      getString() const noexcept;
    BinaryData  getBinaryData() const noexcept;
    int         getArraySize() const noexcept;
    JSONVariant getArrayItem(int idx) const noexcept;
    JSONVariant getObjectItem(const char* key) const noexcept;
    JSONVariant operator[](int idx) const noexcept;
    JSONVariant operator[](const char* key) const noexcept;

    void pushArrayItem(const JSONVariant& var) noexcept;
    void setArrayItem(int idx, const JSONVariant& var) noexcept;
    void insertArrayItem(int idx, const JSONVariant& var) noexcept;
    void setObjectItem(const char* key, const JSONVariant& var) noexcept;

    JSONVariant sliceArray(int start, int end = -1) const noexcept
    {
        return ::sliceVariantArray(*this, start, end);
    }

    JSONVariant& operator+=(const JSONVariant& other) noexcept
    {
        return ::joinVariantArrays(*this, other);
    }

    friend JSONVariant operator+(JSONVariant lhs, const JSONVariant& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    operator bool()   const noexcept { return getBoolean(); }
    operator double() const noexcept { return getNumber(); }
    operator String() const noexcept { return getString(); }

    String toJSON(bool format = false) const noexcept;
    static JSONVariant fromJSON(const char* jsonText) noexcept;

private:
    JSONVariant(cJSON* impl) noexcept;

    cJSON* fImpl;

};

END_NAMESPACE_DISTRHO

#endif // JSON_VARIANT_HPP
