/*
 * Apices - Audio Plugins In C++ & ES6
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef enum {
    OPC_SET_BACKGROUND_COLOR,
    OPC_SET_PARENT,
    OPC_SET_SIZE,
    OPC_SET_POSITION,
    OPC_SET_KEYBOARD_FOCUS,
    OPC_NAVIGATE,
    OPC_RUN_SCRIPT,
    OPC_INJECT_SCRIPT,
    OPC_KEY_EVENT,
    OPC_HANDLE_SCRIPT_MESSAGE,
    OPC_HANDLE_LOAD_FINISHED
} helper_opcode_t;

typedef enum {
    ARG_TYPE_NULL,
    ARG_TYPE_FALSE,
    ARG_TYPE_TRUE,
    ARG_TYPE_DOUBLE,
    ARG_TYPE_STRING
} helper_msg_arg_type_t;

typedef struct {
    unsigned width;
    unsigned height;
} helper_size_t;

typedef struct {
    int x;
    int y;
} helper_pos_t;

typedef enum {
    MOD_SHIFT   = 1 << 0,
    MOD_CONTROL = 1 << 1,
    MOD_ALT     = 1 << 2,
    MOD_SUPER   = 1 << 3
} helper_key_mod_t;

typedef struct {
    char     press;
    unsigned code;
    unsigned hw_code;
    unsigned mod;
} helper_key_t;

#endif  // HELPER_H
