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

#include "extra/JSONVariant.hpp"
#include "extra/Base64.hpp"

USE_NAMESPACE_DISTRHO

JSONVariant::JSONVariant() noexcept
    : fImpl(cJSON_CreateNull())
{}

JSONVariant::JSONVariant(bool b) noexcept
    : fImpl(b ? cJSON_CreateTrue() : cJSON_CreateFalse())
{}

JSONVariant::JSONVariant(double d) noexcept
    : fImpl(cJSON_CreateNumber(d))
{}

JSONVariant::JSONVariant(String s) noexcept
    : fImpl(cJSON_CreateString(s))
{}

JSONVariant::JSONVariant(const BinaryData& data) noexcept
    : fImpl(cJSON_CreateString(String::asBase64(data.data(), data.size())))
{}

JSONVariant::JSONVariant(int32_t i) noexcept
    : fImpl(cJSON_CreateNumber(static_cast<double>(i)))
{}

JSONVariant::JSONVariant(uint32_t i) noexcept
    : fImpl(cJSON_CreateNumber(static_cast<double>(i)))
{}

JSONVariant::JSONVariant(float f) noexcept
    : fImpl(cJSON_CreateNumber(static_cast<double>(f)))
{}

JSONVariant::JSONVariant(const char* s) noexcept
    : fImpl(cJSON_CreateString(s))
{}

JSONVariant::JSONVariant(std::initializer_list<KeyValue> items) noexcept
    : fImpl(cJSON_CreateObject())
{
    for (std::initializer_list<KeyValue>::const_iterator it = items.begin();
            it != items.end(); ++it) {
        setObjectItem(it->first, it->second);
    }
}

JSONVariant::JSONVariant(std::initializer_list<JSONVariant> items) noexcept
    : fImpl(cJSON_CreateArray())
{
    for (std::initializer_list<JSONVariant>::const_iterator it = items.begin();
            it != items.end(); ++it) {
        pushArrayItem(*it);
    }
}

JSONVariant::~JSONVariant()
{
    if (fImpl != nullptr) {
        cJSON_Delete(fImpl);
    }

    fImpl = nullptr;
}

JSONVariant::JSONVariant(const JSONVariant& var) noexcept
    : fImpl(cJSON_Duplicate(var.fImpl, true))
{}

JSONVariant& JSONVariant::operator=(const JSONVariant& var) noexcept
{
    if (fImpl != nullptr) {
        cJSON_Delete(fImpl);
    }

    fImpl = cJSON_Duplicate(var.fImpl, true);

    return *this;
}

JSONVariant::JSONVariant(JSONVariant&& var) noexcept
{
    fImpl = var.fImpl;
    var.fImpl = nullptr;
}

JSONVariant& JSONVariant::operator=(JSONVariant&& var) noexcept
{
    if (this != &var) {
        if (fImpl != nullptr) {
            cJSON_Delete(fImpl);
        }

        fImpl = var.fImpl;
        var.fImpl = nullptr;
    }

   return *this;
}

JSONVariant JSONVariant::createObject(std::initializer_list<KeyValue> items) noexcept
{
    return JSONVariant(items);
}

JSONVariant JSONVariant::createArray(std::initializer_list<JSONVariant> items) noexcept
{
    return JSONVariant(items);
}

bool JSONVariant::isNull() const noexcept
{
    return cJSON_IsNull(fImpl);
}

bool JSONVariant::isBoolean() const noexcept
{
    return cJSON_IsBool(fImpl);
}

bool JSONVariant::isNumber() const noexcept
{
    return cJSON_IsNumber(fImpl);
}

bool JSONVariant::isString() const noexcept
{
    return cJSON_IsString(fImpl);
}

bool JSONVariant::isBinaryData() const noexcept
{
    if (! isString()) {
        return false;
    }

    String s = getString();
    size_t len = s.length();
    const char *data = s.buffer();

    for (size_t i = 0; i < len; ++i) {
        if ( ! ((data[i] >= 'A') && (data[i] <= 'Z'))
            || ((data[i] >= 'a') && (data[i] <= 'z'))
            || ((data[i] >= '0') && (data[i] <= '9'))
            ||  (data[i] == '+')
            ||  (data[i] == '/')
            ||  (data[i] == '=') ) {
            return false;
        }

    }

    return true;
}

bool JSONVariant::isArray() const noexcept
{
    return cJSON_IsArray(fImpl);
}

bool JSONVariant::isObject() const noexcept
{
    return cJSON_IsObject(fImpl);
}

String JSONVariant::asString() const noexcept
{
    return toJSON();
}

bool JSONVariant::getBoolean() const noexcept
{
    return cJSON_IsTrue(fImpl);
}

double JSONVariant::getNumber() const noexcept
{
    return cJSON_GetNumberValue(fImpl);
}

String JSONVariant::getString() const noexcept
{
    return String(cJSON_GetStringValue(fImpl));
}

BinaryData JSONVariant::getBinaryData() const noexcept
{
    return d_getChunkFromBase64String(cJSON_GetStringValue(fImpl));
}

int JSONVariant::getArraySize() const noexcept
{
    return cJSON_GetArraySize(fImpl);
}

JSONVariant JSONVariant::getArrayItem(int idx) const noexcept
{
    return JSONVariant(cJSON_Duplicate(cJSON_GetArrayItem(fImpl, idx), true));
}

JSONVariant JSONVariant::getObjectItem(const char* key) const noexcept
{
    return JSONVariant(cJSON_Duplicate(cJSON_GetObjectItem(fImpl, key), true));
}

JSONVariant JSONVariant::operator[](int idx) const noexcept
{
    return getArrayItem(idx);
}

JSONVariant JSONVariant::operator[](const char* key) const noexcept
{
    return getObjectItem(key);
}

void JSONVariant::pushArrayItem(const JSONVariant& value) noexcept
{
    cJSON_AddItemToArray(fImpl, cJSON_Duplicate(value.fImpl, true));
}

void JSONVariant::setArrayItem(int idx, const JSONVariant& value) noexcept
{
    cJSON_ReplaceItemInArray(fImpl, idx, cJSON_Duplicate(value.fImpl, true));
}

void JSONVariant::insertArrayItem(int idx, const JSONVariant& value) noexcept
{
    cJSON_InsertItemInArray(fImpl, idx, cJSON_Duplicate(value.fImpl, true));
}

void JSONVariant::setObjectItem(const char* key, const JSONVariant& value) noexcept
{
    if (cJSON_HasObjectItem(fImpl, key)) {
        cJSON_ReplaceItemInObject(fImpl, key, cJSON_Duplicate(value.fImpl, true));
    } else {
        cJSON_AddItemToObject(fImpl, key, cJSON_Duplicate(value.fImpl, true));
    }
}

String JSONVariant::toJSON(bool format) const noexcept
{
    char* s = format ? cJSON_Print(fImpl) : cJSON_PrintUnformatted(fImpl);
    String jsonText = String(s);
    cJSON_free(s);

    return jsonText;
}

JSONVariant JSONVariant::fromJSON(const char* jsonText) noexcept
{
    return JSONVariant(cJSON_Parse(jsonText));
}

JSONVariant::JSONVariant(cJSON* impl) noexcept
    : fImpl(impl)
{}
