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

#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef enum {
    OP_CREATE_VIEW,
    OP_SET_BACKGROUND_COLOR,
    OP_SET_SIZE,
    OP_SET_KEYBOARD_FOCUS,
    OP_NAVIGATE,
    OP_RUN_SCRIPT,
    OP_INJECT_SCRIPT,
    OP_HANDLE_SCRIPT_MESSAGE,
    OP_HANDLE_LOAD_FINISHED,
    OP_QUIT
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

#endif  // HELPER_H
