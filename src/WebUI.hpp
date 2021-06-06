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

#include "DistrhoUI.hpp"

#ifdef DISTRHO_OS_LINUX
#include "arch/linux/ExternalGtkWebView.hpp"
#endif
#ifdef DISTRHO_OS_MAC
#include "arch/macos/CocoaWebView.hpp"
#endif
#ifdef DISTRHO_OS_WINDOWS
#include "arch/windows/EdgeWebView.hpp"
#endif

START_NAMESPACE_DISTRHO

class WebUI : public UI, private WebViewEventHandler
{
public:
    WebUI();
    ~WebUI() {};

    void onDisplay() override;

    void parameterChanged(uint32_t index, float value) override;

protected:
    void onResize(const ResizeEvent& ev) override;

private:
    void handleWebViewLoadFinished() override;
    void handleWebViewScriptMessage(ScriptMessageArguments& args) override;

    WEBVIEW_CLASS fWebView;
    uintptr_t     fParentWindowId;

};

END_NAMESPACE_DISTRHO

#endif  // WEBUI_HPP
