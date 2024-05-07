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

#ifndef BSON_VARIANT_HPP
#define BSON_VARIANT_HPP

#if !defined(DPF_WEBUI_SUPPORT_BSON)
# error BSONVariant requires BSON support enabled by DPF_WEBUI_SUPPORT_BSON
#endif

#include <initializer_list>

#include "distrho/extra/String.hpp"
#include "bson/bson.h"

#include "VariantUtil.hpp"

START_NAMESPACE_DISTRHO

class BSONVariant
{
public:
    BSONVariant() noexcept;
    BSONVariant(bool b) noexcept;
    BSONVariant(double d) noexcept;
    BSONVariant(String s) noexcept;
    BSONVariant(const BinaryData& data) noexcept;

    // Convenience constructors for plugin code
    BSONVariant(int32_t i) noexcept;
    BSONVariant(uint32_t i) noexcept;
    BSONVariant(float f) noexcept;
    BSONVariant(const char* s) noexcept;

    typedef std::pair<const char*,BSONVariant> KeyValue;
    BSONVariant(std::initializer_list<KeyValue> items) noexcept;
    BSONVariant(std::initializer_list<BSONVariant> items) noexcept;

    ~BSONVariant();

    BSONVariant(const BSONVariant& var) noexcept;
    BSONVariant& operator=(const BSONVariant& var) noexcept;
    BSONVariant(BSONVariant&& var) noexcept;
    BSONVariant& operator=(BSONVariant&& var) noexcept;

    static BSONVariant createObject(std::initializer_list<KeyValue> items = {}) noexcept;
    static BSONVariant createArray(std::initializer_list<BSONVariant> items = {}) noexcept;

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
    BSONVariant getArrayItem(int idx) const noexcept;
    BSONVariant getObjectItem(const char* key) const noexcept;
    BSONVariant operator[](int idx) const noexcept;
    BSONVariant operator[](const char* key) const noexcept;

    void pushArrayItem(const BSONVariant& var) noexcept;
    void setArrayItem(int idx, const BSONVariant& var) noexcept;
    void insertArrayItem(int idx, const BSONVariant& var) noexcept;
    void setObjectItem(const char* key, const BSONVariant& var) noexcept;

    BSONVariant sliceArray(int start, int end = -1) const noexcept
    {
        return ::sliceVariantArray(*this, start, end);
    }

    BSONVariant& operator+=(const BSONVariant& other) noexcept
    {
        return ::joinVariantArrays(*this, other);
    }

    friend BSONVariant operator+(BSONVariant lhs, const BSONVariant& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    operator bool()   const noexcept { return getBoolean(); }
    operator double() const noexcept { return getNumber(); }
    operator String() const noexcept { return getString(); }

    BinaryData toBSON() const noexcept;
    static BSONVariant fromBSON(const BinaryData& data, bool asArray) noexcept;
    
    String toJSON(bool extended = false, bool canonical = false) const noexcept;
    static BSONVariant fromJSON(const char* jsonText) noexcept;

private:
    BSONVariant(bson_type_t type, bson_t* array) noexcept;

    void copy(const BSONVariant& var) noexcept;
    void move(BSONVariant&& var) noexcept;
    void destroy() noexcept;
    
    static BSONVariant get(const bson_t* bson, const char* key) noexcept;
    static void        set(bson_t* bson, const char* key, const BSONVariant& var) noexcept;

    bson_type_t fType;

    union {
        bool        fBool;
        int32_t     fInt;
        double      fDouble;
        char*       fString;
        BinaryData* fData;
        bson_t*     fDocument;
    };

};

END_NAMESPACE_DISTRHO

#endif // BSON_VARIANT_HPP
