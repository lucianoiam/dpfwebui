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

#include <stdexcept>
#include <string>

#include "extra/BSONVariant.hpp"

USE_NAMESPACE_DISTRHO

BSONVariant::BSONVariant() noexcept
    : fType(BSON_TYPE_NULL)
    , fDocument(nullptr)
{}

BSONVariant::BSONVariant(bool b) noexcept
    : fType(BSON_TYPE_BOOL)
    , fBool(b)
{}

BSONVariant::BSONVariant(double d) noexcept
    : fType(BSON_TYPE_DOUBLE)
    , fDouble(d)
{}

BSONVariant::BSONVariant(String s) noexcept
    : fType(BSON_TYPE_UTF8)
{
    fString = new char[s.length() + 1];
    std::strcpy(fString, s.buffer());
}

BSONVariant::BSONVariant(const BinaryData& data) noexcept
    : fType(BSON_TYPE_BINARY)
{
    fData = new BinaryData(data.begin(), data.end());
}

BSONVariant::BSONVariant(int32_t i) noexcept
    : fType(BSON_TYPE_INT32)
    , fInt(i)
{}

BSONVariant::BSONVariant(uint32_t i) noexcept
    : fType(BSON_TYPE_INT32)
    , fInt(static_cast<int32_t>(i))
{}

BSONVariant::BSONVariant(float f) noexcept
    : fType(BSON_TYPE_DOUBLE)
    , fDouble(static_cast<double>(f))
{}

BSONVariant::BSONVariant(const char* s) noexcept
    : fType(BSON_TYPE_UTF8)
{
    fString = new char[std::strlen(s) + 1];
    std::strcpy(fString, s);
}

BSONVariant::BSONVariant(std::initializer_list<KeyValue> items) noexcept
    : fType(BSON_TYPE_DOCUMENT)
    , fDocument(bson_new())
{
    for (std::initializer_list<KeyValue>::const_iterator it = items.begin();
            it != items.end(); ++it) {
        setObjectItem(it->first, it->second);
    }
}

BSONVariant::BSONVariant(std::initializer_list<BSONVariant> items) noexcept
    : fType(BSON_TYPE_ARRAY)
    , fDocument(bson_new())
{
    for (std::initializer_list<BSONVariant>::const_iterator it = items.begin();
            it != items.end(); ++it) {
        pushArrayItem(*it);
    }
}

BSONVariant::~BSONVariant()
{
    destroy();
}

BSONVariant::BSONVariant(const BSONVariant& var) noexcept
{
    copy(var);
}

BSONVariant& BSONVariant::operator=(const BSONVariant& var) noexcept
{
    destroy();
    copy(var);

    return *this;
}

BSONVariant::BSONVariant(BSONVariant&& var) noexcept
{
    move(std::move(var));
}

BSONVariant& BSONVariant::operator=(BSONVariant&& var) noexcept
{
    if (this != &var) {
        destroy();
        move(std::move(var));
    }

   return *this;
}

BSONVariant BSONVariant::createObject(std::initializer_list<KeyValue> items) noexcept
{
    return BSONVariant(items);
}

BSONVariant BSONVariant::createArray(std::initializer_list<BSONVariant> items) noexcept
{
    return BSONVariant(items);
}

bool BSONVariant::isNull() const noexcept
{
    return fType == BSON_TYPE_NULL;
}

bool BSONVariant::isBoolean() const noexcept
{
    return fType == BSON_TYPE_BOOL;
}

bool BSONVariant::isNumber() const noexcept
{
    return (fType == BSON_TYPE_INT32) || (fType == BSON_TYPE_DOUBLE);
}

bool BSONVariant::isString() const noexcept
{
    return fType == BSON_TYPE_UTF8;
}

bool BSONVariant::isBinaryData() const noexcept
{
    return fType == BSON_TYPE_BINARY;
}

bool BSONVariant::isArray() const noexcept
{
    return fType == BSON_TYPE_ARRAY;
}

bool BSONVariant::isObject() const noexcept
{
    return fType == BSON_TYPE_DOCUMENT;
}

String BSONVariant::asString() const noexcept
{
    switch (fType) {
        case BSON_TYPE_NULL:
            return String("null");
        case BSON_TYPE_BOOL:
            return String(fBool ? "true" : "false");
        case BSON_TYPE_INT32:
            return String(fInt);
        case BSON_TYPE_INT64:
        case BSON_TYPE_DOUBLE:
            return String(fDouble);
        case BSON_TYPE_UTF8:
            return String(fString);
        case BSON_TYPE_BINARY:
            return String("[BinaryData]");
        case BSON_TYPE_ARRAY:
        case BSON_TYPE_DOCUMENT:
            return String("[Object]");
        default:
            break;
    }

    return String();
}

bool BSONVariant::getBoolean() const noexcept
{
    return fString;
}

double BSONVariant::getNumber() const noexcept
{
    if (fType == BSON_TYPE_INT32) {
        return static_cast<double>(fInt);
    }

    if (fType == BSON_TYPE_DOUBLE) {
        return fDouble;
    }

    return 0;
}

String BSONVariant::getString() const noexcept
{
    return String(fString);
}

BinaryData BSONVariant::getBinaryData() const noexcept
{
    return *fData;
}

int BSONVariant::getArraySize() const noexcept
{
    return bson_count_keys(fDocument);
}

BSONVariant BSONVariant::getArrayItem(int idx) const noexcept
{
    return get(fDocument, String(idx).buffer());
}

BSONVariant BSONVariant::getObjectItem(const char* key) const noexcept
{
    return get(fDocument, key);
}

BSONVariant BSONVariant::operator[](int idx) const noexcept
{
    return getArrayItem(idx);
}

BSONVariant BSONVariant::operator[](const char* key) const noexcept
{
    return get(fDocument, key);
}

void BSONVariant::pushArrayItem(const BSONVariant& var) noexcept
{
    String key(bson_count_keys(fDocument));
    set(fDocument, key.buffer(), var);
}

void BSONVariant::setArrayItem(int idx, const BSONVariant& var) noexcept
{
    String key(idx);
    set(fDocument, key.buffer(), var);
}

void BSONVariant::insertArrayItem(int idx, const BSONVariant& var) noexcept
{
    if (fDocument == nullptr) {
        return;
    }

    bson_iter_t iter;

    if (! bson_iter_init(&iter, fDocument)) {
        return;
    }

    int keyCount = static_cast<int>(bson_count_keys(fDocument));

    if (idx > keyCount) {
        return;
    }

    if (idx == keyCount) {
        pushArrayItem(var);
        return;
    }

    bson_t* newArr = bson_new();
    int newIdx;

    while (bson_iter_next(&iter)) {
        try {
            newIdx = std::stoi(bson_iter_key(&iter));
        } catch (std::invalid_argument const& ex) {
            bson_destroy(newArr);
            return;
        }

        if (newIdx >= idx) {
            newIdx++;
        }

        bson_append_iter(newArr, String(newIdx).buffer(), -1, &iter);
    }

    bson_destroy(fDocument);
    fDocument = newArr;

    set(fDocument, String(idx).buffer(), var);
}

void BSONVariant::setObjectItem(const char* key, const BSONVariant& var) noexcept
{
    set(fDocument, key, var);
}

BinaryData BSONVariant::toBSON() const noexcept
{
    if (fDocument == nullptr) {
        return BinaryData();
    }

    const uint8_t* data = bson_get_data(fDocument);   
    
    return BinaryData(data, data + fDocument->len);
}

BSONVariant BSONVariant::fromBSON(const BinaryData& data, bool asArray) noexcept
{
    bson_t* document = bson_new_from_data(data.data(), data.size());

    if (document == nullptr) {
        return BSONVariant();
    }

    return BSONVariant(asArray ? BSON_TYPE_ARRAY : BSON_TYPE_DOCUMENT, document);
}

String BSONVariant::toJSON(bool extended, bool canonical) const noexcept
{
    if (fDocument == nullptr) {
        return asString();
    }

    if (extended) {
        if (canonical) {
            return String(bson_as_canonical_extended_json(fDocument, nullptr));
        } else {
            return String(bson_as_relaxed_extended_json(fDocument, nullptr));
        }
    } else {
        return String(bson_as_json(fDocument, nullptr));
    }
}

BSONVariant BSONVariant::fromJSON(const char* jsonText) noexcept
{
    bson_t* doc = bson_new_from_json(reinterpret_cast<const uint8_t*>(jsonText),
                                        -1, nullptr);
    bson_type_t type = doc != nullptr ? BSON_TYPE_DOCUMENT : BSON_TYPE_EOD;

    return BSONVariant(type, doc);
}

BSONVariant::BSONVariant(bson_type_t type, bson_t* document) noexcept
    : fType(type)
    , fDocument(document)
{}

void BSONVariant::copy(const BSONVariant& var) noexcept
{
    fType = var.fType;

    switch (var.fType) {
        case BSON_TYPE_BOOL:
            fBool = var.fBool;
            break;
        case BSON_TYPE_INT32:
            fInt = var.fInt;
            break;
        case BSON_TYPE_DOUBLE:
            fDouble = var.fDouble;
            break;
        case BSON_TYPE_UTF8:
            fString = new char[std::strlen(var.fString) + 1];
            std::strcpy(fString, var.fString);
            break;
        case BSON_TYPE_BINARY:
            fData = new BinaryData(var.fData->begin(), var.fData->end());
            break;
        case BSON_TYPE_ARRAY:
        case BSON_TYPE_DOCUMENT:
            fDocument = bson_copy(var.fDocument);
            break;
        default:
            break;
    }
}

void BSONVariant::move(BSONVariant&& var) noexcept
{
    fType = var.fType;
    fDocument = var.fDocument;
    var.fType = BSON_TYPE_EOD;
    var.fDocument = nullptr;
}

void BSONVariant::destroy() noexcept
{
    switch (fType) {
        case BSON_TYPE_UTF8:
            delete[] fString;
            break;
        case BSON_TYPE_BINARY:
            delete fData;
            break;
        case BSON_TYPE_ARRAY:
        case BSON_TYPE_DOCUMENT:
            bson_destroy(fDocument);
            break;
        default:
            break;
    }
}

BSONVariant BSONVariant::get(const bson_t* bson, const char* key) noexcept
{
    bson_iter_t iter;

    if ((bson == nullptr) || !bson_iter_init(&iter, bson)
            || !bson_iter_find(&iter, key)) {
        return BSONVariant();
    }

    BSONVariant v;
    bson_type_t type = bson_iter_type(&iter);

    switch (type) {
        case BSON_TYPE_NULL:
            break;
        case BSON_TYPE_BOOL:
            v = BSONVariant(bson_iter_bool(&iter));
            break;
        case BSON_TYPE_INT32:
            v = BSONVariant(bson_iter_int32(&iter));
            break;
        case BSON_TYPE_INT64:
        case BSON_TYPE_DOUBLE:
            v = BSONVariant(bson_iter_as_double(&iter));
            break;
        case BSON_TYPE_UTF8:
            v = BSONVariant(bson_iter_utf8(&iter, nullptr));
            break;
        case BSON_TYPE_BINARY: {
            uint32_t len;
            const uint8_t* data;
            bson_iter_binary(&iter, nullptr, &len, &data);
            v = BSONVariant(BinaryData(data, data + static_cast<size_t>(len)));
            break;
        }
        case BSON_TYPE_ARRAY:
        case BSON_TYPE_DOCUMENT: {
            uint32_t size;
            const uint8_t *data;
            bson_iter_array(&iter, &size, &data);
            v = BSONVariant(type, bson_new_from_data(data, static_cast<size_t>(size)));
            break;
        }
        default:
            break;
    }

    return v;
}

void BSONVariant::set(bson_t* bson, const char* key, const BSONVariant& var) noexcept
{
    if (bson == nullptr) {
        return;
    }

    switch (var.fType) {
        case BSON_TYPE_NULL:
            bson_append_null(bson, key, -1);
            break;
        case BSON_TYPE_BOOL:
            bson_append_bool(bson, key, -1, var.fBool);
            break;
        case BSON_TYPE_INT32:
            bson_append_int32(bson, key, -1, var.fInt);
            break;
        case BSON_TYPE_DOUBLE:
            bson_append_double(bson, key, -1, var.fDouble);
            break;
        case BSON_TYPE_UTF8:
            bson_append_utf8(bson, key, -1, var.fString, -1);
            break;
        case BSON_TYPE_BINARY:
            bson_append_binary(bson, key, -1, BSON_SUBTYPE_BINARY, var.fData->data(),
                                static_cast<uint32_t>(var.fData->size()));
            break;
        case BSON_TYPE_ARRAY:
            bson_append_array(bson, key, -1, var.fDocument);
            break;
        case BSON_TYPE_DOCUMENT:
            bson_append_document(bson, key, -1, var.fDocument);
            break;
        default:
            break;
    }
}
