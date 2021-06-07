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
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "WebUI.hpp"
#include "Window.hpp"

#include "base/Platform.hpp"
#include "base/macro.h"

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebUI;
}

WebUI::WebUI()
    : fWebView(*this)
    , fParentWindowId(0)
{
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * DISTRHO_UI_INITIAL_WIDTH, scaleFactor * DISTRHO_UI_INITIAL_HEIGHT);
#if defined(DISTRHO_UI_BACKGROUND_COLOR) && defined(DGL_OPENGL)
    glClearColor(UNPACK_RGBA(DISTRHO_UI_BACKGROUND_COLOR, GLfloat));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    fWebView.resize(getSize());
    fWebView.navigate("file://" + platform::getResourcePath() + "/index.html");
}

void WebUI::onDisplay()
{
    const Window& window = getParentWindow();
#if defined(DISTRHO_UI_BACKGROUND_COLOR) && defined(DGL_CAIRO)
    cairo_t* cr = window.getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, UNPACK_RGBA(DISTRHO_UI_BACKGROUND_COLOR, double));
    cairo_paint(cr);
#endif
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = window.getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        fWebView.reparent(fParentWindowId);
    }
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    (void)index;
    (void)value;
}

void WebUI::onResize(const ResizeEvent& ev)
{
    fWebView.resize(ev.size);
}

void WebUI::handleWebViewLoadFinished()
{
    // TODO - send state
}

void WebUI::handleWebViewScriptMessage(ScriptMessageArguments& args)
{
    if (DISTRHO_SAFE_GET_SV_ARG_AS_STRING(args) != "DPF") {
        return;
    }
    DISTRHO_SAFE_POP_SV_ARG(args);
    String method = DISTRHO_SAFE_GET_SV_ARG_AS_STRING(args);
    DISTRHO_SAFE_POP_SV_ARG(args);
    if (method == "editParameter") {
        uint32_t index = static_cast<uint32_t>(DISTRHO_SAFE_GET_SV_ARG_AS_DOUBLE(args));
        DISTRHO_SAFE_POP_SV_ARG(args);
        bool started = static_cast<bool>(DISTRHO_SAFE_GET_SV_ARG_AS_BOOL(args));
        DISTRHO_SAFE_POP_SV_ARG(args);
        editParameter(index, started);
        ::printf("Successful call to editParameter(%d, %s)\n", index, started ? "true" : "false");
    } else if (method == "setParameterValue") {
        // uint32_t index, float value
#if DISTRHO_PLUGIN_WANT_STATE
    } else if (method == "setState") {
        // const char* key, const char* value
#endif
    }
}
