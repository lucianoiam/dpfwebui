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

#include "Runtime.hpp"
#include "log.h"

USE_NAMESPACE_DISTRHO

#if defined(DISTRHO_OS_LINUX) || defined(DISTRHO_OS_MAC)

#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#ifdef DISTRHO_OS_LINUX
#include <linux/limits.h>
#elif DISTRHO_OS_MAC
#include <sys/syslimits.h>
#endif
#include <iostream>

char _dummy; // for dladdr()

String runtime::getBinaryDirectoryPath()
{
    char path[PATH_MAX];
    ::strcpy(path, getBinaryPath());
    return String(dirname(path));
}

String runtime::getBinaryPath()
{
    // DISTRHO_PLUGIN_TARGET_* macros are not available here
    // Is there a better way to differentiate we are being called from library or executable?
    String libPath = getSharedLibraryPath();     // path never empty even if running standalone
    void *handle = ::dlopen(libPath, RTLD_LAZY); // returns non-null on macOS also for standalone
    if (handle) {
        ::dlclose(handle);
        std::cout << libPath << std::endl;
        return libPath;
    }
    return getExecutablePath();
}

String runtime::getSharedLibraryPath()
{
    Dl_info dl_info;
    if (::dladdr((void *)&_dummy, &dl_info) == 0) {
        LOG_STDERR(::dlerror());
        return String();
    }
    return String(dl_info.dli_fname);
}

#ifdef DISTRHO_OS_LINUX

String runtime::getExecutablePath()
{
    char path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        LOG_STDERR_ERRNO("Could not read executable path");
        return String();
    }
    return String(path);
}

#elif DISTRHO_OS_MAC

String runtime::getExecutablePath()
{
    return getSharedLibraryPath();  // does the trick on macOS
}

#endif

#endif // DISTRHO_OS_LINUX || DISTRHO_OS_MAC



/*
String EdgeWebViewUI::getSharedLibraryDirectoryPath()
{
    WCHAR dllPath[MAX_PATH];

    if (GetModuleFileName((HINSTANCE)&__ImageBase, dllPath, sizeof(dllPath)) == 0) {
        LOG_STDERR_INT("Failed GetModuleFileName() call", GetLastError());
        return String();
    }

    PathRemoveFileSpec(dllPath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string path = converter.to_bytes(dllPath); 
    std::replace(path.begin(), path.end(), '\\', '/');

    return String(path.c_str());
}
*/
