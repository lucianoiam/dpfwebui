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

#ifndef MACRO_H
#define MACRO_H

#include <errno.h>

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define XSTR(s) STR(s)
#define STR(s) #s

#define DISTRHO_UNPACK_RGBA_NORM(c,t)  ( c >> 24)               / (t)(255), \
                                       ((c & 0x00ff0000) >> 16) / (t)(255), \
                                       ((c & 0x0000ff00) >> 8 ) / (t)(255), \
                                       ( c & 0x000000ff)        / (t)(255)
#ifdef __cplusplus

#include "distrho/DistrhoUtils.hpp"

#define HIPHOP_LOG_STDERR(msg)        d_stderr("%s : %s", __PRETTY_FUNCTION__, msg); fflush(stderr);
#define HIPHOP_LOG_STDERR_INT(msg,n)  d_stderr("%s : %s - %x", __PRETTY_FUNCTION__, msg, n); fflush(stderr);
#define HIPHOP_LOG_STDERR_ERRNO(msg)  d_stderr("%s : %s - %s", __PRETTY_FUNCTION__, msg, strerror(errno)); fflush(stderr);
#define HIPHOP_LOG_STDERR_COLOR(msg)  d_stderr2("%s : %s", __PRETTY_FUNCTION__, msg); fflush(stderr);

#else

#include <stdio.h>

#define HIPHOP_LOG_STDERR(msg)        fprintf(stderr, "%s : %s\n", __PRETTY_FUNCTION__, msg);
#define HIPHOP_LOG_STDERR_INT(msg,n)  fprintf(stderr, "%s : %s - %x\n", __PRETTY_FUNCTION__, msg, (unsigned int)n);
#define HIPHOP_LOG_STDERR_ERRNO(msg)  fprintf(stderr, "%s : %s - %s\n", __PRETTY_FUNCTION__, msg, strerror(errno));
#define HIPHOP_LOG_STDERR_COLOR(msg)  fprintf(stderr, "\x1b[31m%s : %s\x1b[0m\n", __PRETTY_FUNCTION__, msg);

#endif  // __cplusplus

#endif  // MACRO_H
