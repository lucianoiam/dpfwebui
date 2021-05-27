/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LOG_H
#define LOG_H

#include <errno.h>

#ifdef __cplusplus

#define LOG_STDERR(msg)             d_stderr("%s : %s", __PRETTY_FUNCTION__, msg);
#define LOG_STDERR_INT(msg,n)		d_stderr("%s : %s - %x", __PRETTY_FUNCTION__, msg, n);
#define LOG_STDERR_COLOR(msg)       d_stderr2("%s : %s", __PRETTY_FUNCTION__, msg);
#define LOG_STDERR_ERRNO(msg)       d_stderr("%s : %s - %s", __PRETTY_FUNCTION__, msg, strerror(errno));
#define LOG_STDERR_ERRNO_INT(msg,n) d_stderr("%s : %s %d - %s", __PRETTY_FUNCTION__, msg, n, strerror(errno));

#else

#include <stdio.h>

#define LOG_STDERR(msg)             fprintf(stderr, "%s : %s\n", __PRETTY_FUNCTION__, msg);
#define LOG_STDERR_INT(msg,n)		fprintf(stderr, "%s : %s - %x\n", __PRETTY_FUNCTION__, msg, n);
#define LOG_STDERR_COLOR(msg)       fprintf(stderr, "\x1b[31m%s : %s\x1b[0m\n", __PRETTY_FUNCTION__, msg);
#define LOG_STDERR_ERRNO(msg)       fprintf(stderr, "%s : %s - %s\n", __PRETTY_FUNCTION__, msg, strerror(errno));
#define LOG_STDERR_ERRNO_INT(msg,n) fprintf(stderr, "%s : %s %d - %s\n", __PRETTY_FUNCTION__, msg, n, strerror(errno));

#endif

#endif  // LOG_H
