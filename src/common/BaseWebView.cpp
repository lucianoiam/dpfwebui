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

#include "BaseWebView.hpp"

#define JS_DISABLE_CONTEXT_MENU \
    "document.body.setAttribute('oncontextmenu', 'event.preventDefault()');"

#define JS_DISABLE_ZOOM \
    "document.head.insertAdjacentHTML('beforeend', '<meta name=\"viewport\"" \
        " content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\" />');"

#define CSS_DISABLE_SELECTION \
    "body {" \
    "   -webkit-user-select: none;" \
    "}"

void BaseWebView::loadFinished()
{
    runScript(String(JS_DISABLE_CONTEXT_MENU));
    runScript(String(JS_DISABLE_ZOOM));
    addStylesheet(String(CSS_DISABLE_SELECTION));
}

void BaseWebView::addStylesheet(String source)
{
    String js;
    js += "const style = document.createElement('style');"
          "style.innerHTML = '" + source + "';"
          "document.head.appendChild(style);";
    runScript(js);
}
