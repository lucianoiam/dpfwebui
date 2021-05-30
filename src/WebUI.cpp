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

#include "RuntimePath.hpp"

USE_NAMESPACE_DISTRHO

WebUI::WebUI()
    : UI(DISTRHO_UI_INITIAL_WIDTH, DISTRHO_UI_INITIAL_HEIGHT)
    , fParentWindowId(0)
{
#ifdef DGL_OPENGL
    uint rgba = getBackgroundColor();
    GLfloat r = (rgba >> 24) / 255.f;
    GLfloat g = ((rgba & 0x00ff0000) >> 16) / 255.f;
    GLfloat b = ((rgba & 0x0000ff00) >> 8) / 255.f;
    GLfloat a = (rgba & 0x000000ff) / 255.f;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void WebUI::onDisplay()
{
    const Window& window = getParentWindow();
#ifdef DGL_CAIRO
    uint rgba = getBackgroundColor();
    double r = (rgba >> 24) / 255.f;
    double g = ((rgba & 0x00ff0000) >> 16) / 255.0;
    double b = ((rgba & 0x0000ff00) >> 8) / 255.0;
    double a = (rgba & 0x000000ff) / 255.0;
    cairo_t* cr = window.getGraphicsContext().cairo;
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_paint(cr);
#endif
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = window.getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        reparent(fParentWindowId);
    }
}

String WebUI::getContentUrl()
{
    return "file://" + rtpath::getResourcePath() + "/index.html";
}
