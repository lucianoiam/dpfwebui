/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#ifndef IPC_MESSAGE_H
#define IPC_MESSAGE_H

#include <stdint.h>

typedef enum {
    OP_REALIZE,
    OP_NAVIGATE,
    OP_RUN_SCRIPT,
    OP_INJECT_SHIMS,
    OP_INJECT_SCRIPT,
    OP_SET_SIZE,
    OP_SET_KEYBOARD_FOCUS,
    OP_TERMINATE,
    OP_HANDLE_INIT,
    OP_HANDLE_SCRIPT_MESSAGE,
    OP_HANDLE_LOAD_FINISHED
} msg_opcode_t;

typedef enum {
    ARG_TYPE_NULL,
    ARG_TYPE_FALSE,
    ARG_TYPE_TRUE,
    ARG_TYPE_DOUBLE,
    ARG_TYPE_STRING
} msg_js_arg_type_t;

typedef struct {
    unsigned width;
    unsigned height;
} msg_view_size_t;

typedef struct {
    uintptr_t       parent;
    uint32_t        color;
    msg_view_size_t size;
    char            userAgent[1024];
} msg_view_cfg_t;

#endif  // IPC_MESSAGE_H
