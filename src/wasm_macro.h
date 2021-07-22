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

#ifndef WASM_MACRO_H
#define WASM_MACRO_H

#include "wasm.h"

// Useful macros for reducing Wasmer C interface noise

#define WASM_DEFINE_ARGS_VAL_VEC_1(var,arg0) wasm_val_t var[1] = { arg0 }; \
                                             wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);
#define WASM_DEFINE_ARGS_VAL_VEC_2(var,arg0,arg1) wasm_val_t var[2] = { arg0, arg1 }; \
                                                  wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);

#define WASM_DEFINE_RES_VAL_VEC_1(var) wasm_val_t var[1] = { WASM_INIT_VAL }; \
                                       wasm_val_vec_t var##_val_vec = WASM_ARRAY_VEC(var);

#define own

#define WASM_DECLARE_NATIVE_FUNC(func) static own wasm_trap_t* func(void *env, const wasm_val_vec_t* args, wasm_val_vec_t* results);

#endif  // WASM_MACRO_H
