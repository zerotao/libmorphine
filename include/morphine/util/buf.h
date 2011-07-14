#ifndef __CHNO_BUFFER_H__
#define __CHNO_BUFFER_H__

struct chno_buffer_s;
typedef struct chno_buffer_s chno_buffer_t;

chno_buffer_t * chno_buffer_new(void);
size_t          chno_buffer_length(chno_buffer_t *);
size_t          chno_buffer_drain(chno_buffer_t *, size_t);
int             chno_buffer_add(chno_buffer_t *, void *, size_t);
int             chno_buffer_add_buffer(chno_buffer_t *, chno_buffer_t *);
int             chno_buffer_remove(chno_buffer_t *, void *, size_t);
void            chno_buffer_free(chno_buffer_t *);
void          * chno_buffer_get(chno_buffer_t * b);

#endif /* __BUFFER_H__ */

