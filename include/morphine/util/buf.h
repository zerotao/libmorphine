#ifndef __CHNO_BUFFER_H__
#define __CHNO_BUFFER_H__

struct chno_buffer_s;
typedef struct chno_buffer_s chno_buffer_t;
typedef struct chno_iovec_s  chno_iovec_t;

struct chno_iovec_s {
    size_t iov_len;
    void * iov_data;
};

chno_buffer_t * chno_buffer_new(void);
size_t          chno_buffer_length(chno_buffer_t *);
size_t          chno_buffer_drain(chno_buffer_t *, size_t);
int             chno_buffer_add(chno_buffer_t *, void *, size_t);
int             chno_buffer_add_buffer(chno_buffer_t *, chno_buffer_t *);
int             chno_buffer_remove(chno_buffer_t *, void *, size_t);
void            chno_buffer_free(chno_buffer_t *);
void          * chno_buffer_get(chno_buffer_t * b);
int             chno_buffer_reserve(chno_buffer_t *, size_t);
int             chno_buffer_iovec_add(chno_buffer_t *, chno_iovec_t *, int);

#endif /* __BUFFER_H__ */

