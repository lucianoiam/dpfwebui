/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "WebExamplePlugin.hpp"

USE_NAMESPACE_DISTRHO

Plugin* DISTRHO::createPlugin()
{
    return new WebExamplePlugin;
}

WebExamplePlugin::WebExamplePlugin()
    : Plugin(0, 0, 0)
{}

void WebExamplePlugin::initParameter(uint32_t index, Parameter& parameter)
{
    // unused
    (void)index;
    (void)parameter;
}

float WebExamplePlugin::getParameterValue(uint32_t index) const
{
    return 0;

    // unused
    (void)index;
}

void WebExamplePlugin::setParameterValue(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}

#ifdef DISTRHO_PLUGIN_WANT_STATE

void WebExamplePlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    // unused
    (void)index;
    (void)stateKey;
    (void)defaultStateValue;
}

void WebExamplePlugin::setState(const char* key, const char* value)
{
    // unused
    (void)key;
    (void)value;
}

#ifdef DISTRHO_PLUGIN_WANT_FULL_STATE

String WebExamplePlugin::getState(const char* key) const
{
    // unused
    (void)key;
    return String();
}

#endif // DISTRHO_PLUGIN_WANT_FULL_STATE

#endif // DISTRHO_PLUGIN_WANT_STATE

void WebExamplePlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    memcpy(outputs[0], inputs[0], frames * sizeof(float));
}
