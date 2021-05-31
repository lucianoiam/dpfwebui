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
    ipc_conf_t conf;
    void*      buf;
};

static void ipc_free_buf(ipc_t *ipc)
{
    if (ipc->buf) {
        free(ipc->buf);
        ipc->buf = NULL;
    }
}

ipc_t* ipc_init(const ipc_conf_t *conf)
{
    ipc_t *ipc = malloc(sizeof(ipc_t));
    ipc->conf.fd_r = conf->fd_r;
    ipc->conf.fd_w = conf->fd_w;
    ipc->buf = NULL;
    return ipc;
}

void ipc_destroy(ipc_t *ipc)
{
    ipc_free_buf(ipc);
    free(ipc);
}

int ipc_read(ipc_t *ipc, tlv_t *packet)
{
    if (read(ipc->conf.fd_r, &packet->t, sizeof(packet->t)) != sizeof(packet->t)) {
        return -1;
    }

    if (read(ipc->conf.fd_r, &packet->l, sizeof(packet->l)) != sizeof(packet->l)) {
        return -1;
    }

    ipc_free_buf(ipc);
    
    if (packet->l > 0) {
        ipc->buf = malloc(packet->l);

        if (read(ipc->conf.fd_r, ipc->buf, packet->l) != packet->l) {
            return -1;
        }
        
        packet->v = ipc->buf;
    }

    return 0;
}

int ipc_write(const ipc_t *ipc, const tlv_t *packet)
{
    if (write(ipc->conf.fd_w, &packet->t, sizeof(packet->t)) == -1) {
        return -1;
    }

    if (write(ipc->conf.fd_w, &packet->l, sizeof(packet->l)) == -1) {
        return -1;
    }
    
    if ((packet->l > 0) && (write(ipc->conf.fd_w, packet->v, packet->l) == -1)) {
        return -1;
    }

    return 0;
}

const ipc_conf_t* ipc_get_config(const ipc_t *ipc)
{
    return &ipc->conf;
}
