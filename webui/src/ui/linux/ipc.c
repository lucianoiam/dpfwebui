/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipc.h"

struct priv_ipc_t {
    ipc_conf_t conf;
    void*      buf;
};

struct priv_hdr_t {
    short t;
    int   l;
};

#define HEADER_SIZE sizeof(struct priv_hdr_t)

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

int ipc_read(ipc_t *ipc, tlv_t *pkt)
{
    if (read(ipc->conf.fd_r, pkt, HEADER_SIZE) == -1) {
        fprintf(stderr, "ipc : read() header - errno %d\n", errno);
        return -1;
    }

    ipc_free_buf(ipc);

    if (pkt->l > 0) {
        ipc->buf = malloc(pkt->l);

        if (read(ipc->conf.fd_r, ipc->buf, pkt->l) == -1) {
            fprintf(stderr, "ipc : read() %d bytes - errno %d\n", pkt->l, errno);
            return -1;
        }
        
        pkt->v = ipc->buf;
    }

    return 0;
}

int ipc_write(const ipc_t *ipc, const tlv_t *pkt)
{
    if (write(ipc->conf.fd_w, pkt, HEADER_SIZE) == -1) {
        fprintf(stderr, "ipc : write() header - errno %d\n", errno);
        return -1;
    }

    if ((pkt->l > 0) && (write(ipc->conf.fd_w, pkt->v, pkt->l) == -1)) {
        fprintf(stderr, "ipc : write() %d bytes - errno %d\n", pkt->l, errno);
        return -1;
    }

    return 0;
}

const ipc_conf_t* ipc_get_config(const ipc_t *ipc)
{
    return &ipc->conf;
}
