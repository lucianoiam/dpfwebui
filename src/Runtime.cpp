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
#include "macro.h"

USE_NAMESPACE_DISTRHO

#if defined(DISTRHO_OS_LINUX) || defined(DISTRHO_OS_MAC)

#include <cstring>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>

char _dummy; // for dladdr()

#ifdef DISTRHO_OS_LINUX

#include <linux/limits.h>

String runtime::getExecutablePath()
{
    char path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        LOG_STDERR_ERRNO("Could not determine executable path");
        return String();
    }
    return String(path);
}

#elif DISTRHO_OS_MAC

#include <sys/syslimits.h>

String runtime::getExecutablePath()
{
    return getSharedLibraryPath();  // does the trick on macOS
}

#endif

String runtime::getSharedLibraryPath()
{
    Dl_info dl_info;
    if (::dladdr((void *)&_dummy, &dl_info) == 0) {
        LOG_STDERR(::dlerror());
        return String();
    }
    return String(dl_info.dli_fname);
}

String runtime::getBinaryPath()
{
    // DISTRHO_PLUGIN_TARGET_* macros are not available here
    // Is there a better way to differentiate we are being called from library or executable?
    String libPath = getSharedLibraryPath();     // path never empty even if running standalone
    void *handle = ::dlopen(libPath, RTLD_LAZY); // returns non-null on macOS also for standalone
    if (handle) {
        ::dlclose(handle);
        return libPath;
    }
    return getExecutablePath();
}

String runtime::getBinaryDirectoryPath()
{
    char path[PATH_MAX];
    ::strcpy(path, getBinaryPath());
    return String(::dirname(path));
}

#endif // DISTRHO_OS_LINUX || DISTRHO_OS_MAC


#ifdef DISTRHO_OS_WINDOWS

#include <cstring>
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <shlwapi.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

String getExecutablePath()
{
    // Standalone JACK app on Windows is not currently implemented
    return String();
}

String runtime::getSharedLibraryPath()
{
    char path[MAX_PATH];
    if (::GetModuleFileName((HINSTANCE)&__ImageBase, path, sizeof(path)) == 0) {
        LOG_STDERR_INT("Could not determine DLL path", ::GetLastError());
        return String();
    }
    return String(path);
}

String runtime::getBinaryPath()
{
    return getSharedLibraryPath();
}

String runtime::getBinaryDirectoryPath()
{
    char path[MAX_PATH];
    ::strcpy(path, getBinaryPath());
    ::PathRemoveFileSpec(path);
    return String(path);
}

#endif // DISTRHO_OS_WINDOWS


String runtime::getResourcePath()
{
#ifdef DISTRHO_OS_MAC
    // There is no DISTRHO method for querying plugin format during runtime
    // Anyways the ideal solution is to modify the Makefile and rely on macros
    // Mac VST is the only special case
    char path[PATH_MAX];
    ::strcpy(path, getSharedLibraryPath());
    void *handle = ::dlopen(path, RTLD_NOLOAD);
    if (handle != NULL) {
        void *addr = ::dlsym(handle, "VSTPluginMain");
        ::dlclose(handle);
        if (addr != NULL) {
            return String(::dirname(path)) + "/../Resources";
        }
    }
#endif
    String path = getBinaryDirectoryPath();
#ifdef DISTRHO_OS_WINDOWS
	path.replace('\\', '/');
#endif
    return path + "/" XSTR(BIN_BASENAME) "_resources";
}
