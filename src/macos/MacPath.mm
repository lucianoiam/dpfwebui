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

#import <Foundation/Foundation.h>

#include <cstring>
#include <libgen.h>
#include <sys/stat.h>

#include "DistrhoPluginUtils.hpp"

#include "Path.hpp"
#include "macro.h"

USE_NAMESPACE_DISTRHO

String path::getLibraryPath()
{
    char binPath[PATH_MAX];
    strcpy(binPath, getBinaryFilename());
    String binDirPath(dirname(binPath));
    
    const char* format = getPluginFormatName();

    if (strcmp(format, "LV2") == 0) {
        return binDirPath + "/" + kBundleLibrarySubdirectory;
    } else if ((strcmp(format, "VST2") == 0) || (strcmp(format, "VST3") == 0)) {
        return binDirPath.truncate(binDirPath.rfind('/')) + "/Resources";
    }

    // Standalone
    return binDirPath + "/" + kNoBundleLibrarySubdirectory;
}

String path::getCachesPath()
{
    NSArray* p = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    String path = String([[p lastObject] cStringUsingEncoding:NSUTF8StringEncoding])
        + "/" XSTR(PLUGIN_BIN_BASENAME);
    
    mkdir(path, 0777);

    return path;
}
