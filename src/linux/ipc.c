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

#include <stdlib.h>
#include <unistd.h>

#include "ipc.h"

struct priv_ipc_t {
    int   r_fd;
    int   w_fd;
    void* buf;
};

static void ipc_free_buf(ipc_t *ipc)
{
    if (ipc->buf) {
        free(ipc->buf);
        ipc->buf = NULL;
    }
}

ipc_t* ipc_init(int r_fd, int w_fd)
{
    ipc_t *ipc = malloc(sizeof(ipc_t));
    ipc->r_fd = r_fd;
    ipc->w_fd = w_fd;
    ipc->buf = NULL;
    return ipc;
}

void ipc_destroy(ipc_t *ipc)
{
    ipc_free_buf(ipc);
    free(ipc);
}

int ipc_get_read_fd(ipc_t *ipc)
{
    return ipc->r_fd;
}

int ipc_read(ipc_t *ipc, tlv_t *packet)
{
    if (read(ipc->r_fd, &packet->t, sizeof(packet->t)) == -1) {
        return -1;
    }

    if (read(ipc->r_fd, &packet->l, sizeof(packet->l)) == -1) {
        return -1;
    }

    ipc_free_buf(ipc);
    
    if (packet->l > 0) {
        ipc->buf = malloc(packet->l);

        if (read(ipc->r_fd, ipc->buf, packet->l) == -1) {
            return -1;
        }
        
        packet->v = ipc->buf;
    }

    return 0;
}

int ipc_write(const ipc_t *ipc, const tlv_t *packet)
{
    if (write(ipc->w_fd, &packet->t, sizeof(packet->t)) == -1) {
        return -1;
    }

    if (write(ipc->w_fd, &packet->l, sizeof(packet->l)) == -1) {
        return -1;
    }
    
    if ((packet->l > 0) && (write(ipc->w_fd, packet->v, packet->l) == -1)) {
        return -1;
    }

    return 0;
}
