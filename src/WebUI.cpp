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

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebUI;
}

WebUI::WebUI()
    : UI(800, 600)
    , fParentWindowId(0)
{
	// empty
}

WebUI::~WebUI()
{
    //syslog(LOG_INFO, "%p WebUI::~WebUI()", this);
}

void WebUI::onDisplay()
{
	uintptr_t newParentWindowId = getParentWindow().getWindowId();
	
	if (fParentWindowId != newParentWindowId) {
		fParentWindowId = newParentWindowId;
		fWebView.reparent(fParentWindowId);
	}
}

void WebUI::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}
