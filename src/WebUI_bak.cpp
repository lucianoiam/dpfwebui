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

// TO DO approach:
// Linux: launch process with GTK+WebKit to avoiding host GTK issues, then reparent
// macOS: use system webview
// Windows: use system webview

// TODO 4: there should be an interface to abstract the web views

#include "WebUI.hpp"
#include "Window.hpp"
#include "src/DistrhoDefines.h"

#ifdef DISTRHO_OS_LINUX
// TODO 2: move linux specific code to separate file
#include <syslog.h>
#include <spawn.h>
extern char **environ;
#endif

#ifdef DISTRHO_OS_MAC
// TODO 3: move mac specific code to separate file
#include "macos/WebView.h"
#endif

#ifdef DISTRHO_OS_WINDOWS
// TODO 4: move win specific code to separate file
#include "windows/WebView.h"
#endif

#define CONTENT_URL "https://distrho.sourceforge.io/images/screenshots/distrho-kars.png"

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new WebUI;
}

WebUI::WebUI()
    : UI(800, 600)
{
    //syslog(LOG_INFO, "%p WebUI::WebUI()", this);

    // TODO : some hosts like REAPER recreate the parent window every time
    //        the plugin UI is opened

    // UI and DSP code are completely isolated, pass opaque pointer as the owner
    uintptr_t windowId = getParentWindow().getWindowId();


#ifdef DISTRHO_OS_LINUX
    // TODO 2 - proof of concept
    char strWindowId[sizeof(uintptr_t) + /* 0x + \0 */ 3];
    sprintf(strWindowId, "%lx", (long)windowId);
    pid_t pid;
    const char *argv[] = {"helper", strWindowId, CONTENT_URL, NULL};
    const char* fixmeHardcodedPath = "/home/user/src/dpf-webui/bin/d_dpf_webui_helper";
    int status = posix_spawn(&pid, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);
    syslog(LOG_INFO, "posix_spawn() status %d\n", status);
#endif

}

WebUI::~WebUI()
{
    //syslog(LOG_INFO, "%p WebUI::~WebUI()", this);
}

void WebUI::onDisplay()
{
    //syslog(LOG_INFO, "%p WebUI::onDisplay()", this);

    // There is no guarantee the window ID is kept across onDisplay() calls
    uintptr_t windowId = getParentWindow().getWindowId();
#ifdef DISTRHO_OS_MAC
    createWebView(windowId, CONTENT_URL);
#endif

#ifdef DISTRHO_OS_WINDOWS
    createWebView(windowId, CONTENT_URL);
#endif

}

void WebUI::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}
