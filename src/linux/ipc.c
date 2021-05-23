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

int ipc_read(ipc_t *ipc, ipc_msg_t *msg)
{
    if (read(ipc->r_fd, &msg->opcode, sizeof(msg->opcode)) == -1) {
        return -1;
    }

    if (read(ipc->r_fd, &msg->payload_sz, sizeof(msg->payload_sz)) == -1) {
        return -1;
    }

    ipc_free_buf(ipc);
    
    if (msg->payload_sz > 0) {
        ipc->buf = malloc(msg->payload_sz);

        if (read(ipc->r_fd, ipc->buf, msg->payload_sz) == -1) {
            return -1;
        }
        
        msg->payload = ipc->buf;
    }

    return 0;
}

int ipc_write(const ipc_t *ipc, const ipc_msg_t *msg)
{
    if (write(ipc->w_fd, &msg->opcode, sizeof(msg->opcode)) == -1) {
        return -1;
    }

    if (write(ipc->w_fd, &msg->payload_sz, sizeof(msg->payload_sz)) == -1) {
        return -1;
    }
    
    if ((msg->payload_sz > 0) && (write(ipc->w_fd, msg->payload, msg->payload_sz) == -1)) {
        return -1;
    }

    return 0;
}
