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

#ifndef WEBUI_HPP
#define WEBUI_HPP

#include <cstdint>

#include "DistrhoUI.hpp"
#include "extra/String.hpp"

#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

class WebUI : public UI
{
public:
    WebUI(uint width = DISTRHO_UI_INITIAL_WIDTH, uint height = DISTRHO_UI_INITIAL_HEIGHT);
    virtual ~WebUI() {};

    void onDisplay() override;

#ifdef DISTRHO_UI_BACKGROUND_COLOR
    // Hides UI method that attempts to query host
    uint getBackgroundColor() const noexcept { return DISTRHO_UI_BACKGROUND_COLOR; };
#endif

protected:
    virtual void reparent(uintptr_t parentWindowId) = 0;

    String getContentUrl();

private:
    uintptr_t fParentWindowId;

};

END_NAMESPACE_DISTRHO

#endif  // WEBUI_HPP
