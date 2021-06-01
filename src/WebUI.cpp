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

#include "Platform.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

WebUI::WebUI()
    : fParentWindowId(0)
{
    float scaleFactor = platform::getSystemDisplayScaleFactor();
    setSize(scaleFactor * DISTRHO_UI_INITIAL_WIDTH, scaleFactor * DISTRHO_UI_INITIAL_HEIGHT);
#ifdef DGL_OPENGL
    uint rgba = getBackgroundColor();
    glClearColor(UNPACK_RGBA(rgba, GLfloat));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void WebUI::onDisplay()
{
    const Window& window = getParentWindow();
#ifdef DGL_CAIRO
    cairo_t* cr = window.getGraphicsContext().cairo;
    uint rgba = getBackgroundColor();
    cairo_set_source_rgba(UNPACK_RGBA(rgba, double));
    cairo_paint(cr);
#endif
    // onDisplay() can be called multiple times during lifetime of instance
    uintptr_t newParentWindowId = window.getWindowId();
    if (fParentWindowId != newParentWindowId) {
        fParentWindowId = newParentWindowId;
        reparent(fParentWindowId);
    }
}

String WebUI::getResourcePath()
{
    return platform::getBinaryDirectoryPath().replace('\\', '/')
        + "/" XSTR(BIN_BASENAME) "_resources";
}

String WebUI::getContentUrl()
{
    return "file://" + getResourcePath() + "/index.html";
}
