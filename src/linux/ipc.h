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

#ifndef IPC_H
#define IPC_H

typedef struct priv_ipc_t ipc_t;

typedef struct {
    int fd_r;
    int fd_w;
} ipc_conf_t;

typedef struct {
    short       t;
    int         l;
    const void* v;
} tlv_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

ipc_t*            ipc_init(const ipc_conf_t *conf);
void              ipc_destroy(ipc_t *ipc);
int               ipc_read(ipc_t *ipc, tlv_t *pkt);
int               ipc_write(const ipc_t *ipc, const tlv_t *pkt);
const ipc_conf_t* ipc_get_config(const ipc_t *ipc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // IPC_H
