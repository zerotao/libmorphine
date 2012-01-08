#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "morphine/util/buf.h"

struct chno_buffer_s {
    size_t len;
    size_t max;
    void * buf;
    void * top;
};

chno_buffer_t *
chno_buffer_new(void) {
    chno_buffer_t * b;

    b      = malloc(sizeof(chno_buffer_t));
    b->len = 0;
    b->max = 0;
    b->buf = NULL;
    b->top = NULL;

    return b;
}

void *
chno_buffer_get(chno_buffer_t * b) {
    return b->len ? b->buf : NULL;
}

size_t
chno_buffer_unused_sz(chno_buffer_t * b) {
    return (size_t)(b->max - b->len);
}

size_t
chno_buffer_length(chno_buffer_t * b) {
    return b->len;
}

#define CHNO_BUF_MIN 1024

int
chno_buffer_expand(chno_buffer_t * b, size_t len) {
    size_t rlen;

    if (len < CHNO_BUF_MIN) {
        rlen = CHNO_BUF_MIN;
    } else {
        rlen = len;
    }

    if (b->top == NULL) {
        b->top = b->buf;
    }

    b->buf = realloc(b->buf, b->len + rlen);
    b->top = b->buf;
    b->max = b->len + rlen;

    return 0;
}

size_t
chno_buffer_drain(chno_buffer_t * b, size_t len) {
    if (len == 0) {
        return 0;
    }

    if (len < b->len) {
        b->len -= len;
        b->max -= (size_t)(b->top - b->buf);
        b->buf += len;
    } else {
        b->len = 0;
    }

    return b->len;
}

int
chno_buffer_add(chno_buffer_t * b, void * data, size_t len) {
    if (len > chno_buffer_unused_sz(b)) {
        chno_buffer_expand(b, len);
    }

    memcpy((void *)((size_t)b->buf + b->len), data, len);
    b->len += len;

    return 0;
}

int
chno_buffer_add_buffer(chno_buffer_t * a, chno_buffer_t * b) {
    chno_buffer_add(a, b->buf, b->len);
    chno_buffer_drain(b, b->len);
    return 0;
}

int
chno_buffer_remove(chno_buffer_t * b, void * out, size_t olen) {
    if (olen > b->len) {
        olen = b->len;
    }

    memcpy(out, b->buf, olen);
    chno_buffer_drain(b, olen);
    return 0;
}

int
chno_buffer_reserve(chno_buffer_t * b, size_t len) {
    if (len > chno_buffer_unused_sz(b)) {
        chno_buffer_expand(b, len);
    }

    return 0;
}

size_t
chno_buffer_iovec_len(chno_iovec_t * vec, int n_vec) {
    int    i;
    size_t len;

    len = 0;

    for (i = 0; i < n_vec; i++) {
        len += vec[i].iov_len;
    }

    return len;
}

int
chno_buffer_iovec_add(chno_buffer_t * b, chno_iovec_t * vec, int n_vec) {
    int    i;
    size_t tlen;

    tlen = chno_buffer_iovec_len(vec, n_vec);
    chno_buffer_reserve(b, tlen);

    for (i = 0; i < n_vec; i++) {
        chno_buffer_add(b, vec[i].iov_data, vec[i].iov_len);
    }

    return 0;
}

void
chno_buffer_free(chno_buffer_t * b) {
    if (b->buf) {
        free(b->top);
    }

    free(b);
}

