#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <zt.h>

#include "morphine/util.h"
#include "morphine/base.h"
#include "private.h"

CHNO_M_GENERATE;

chno_t *
chno_new(chno_type_t type) {
    chno_t * mbr = calloc(sizeof(chno_t), 1);

    mbr->type = type;

    return mbr;
}

typedef int (*chno_tbl_iter_cb)(const char *, void *, void *);

static unsigned int
_hash(const char * key) {
    unsigned int h = 0;

    for (; *key; key++) {
        h = 31 * h + *key;
    }

    return h % 16;
}

static struct chno_tbl *
chno_tbl_new(void) {
    struct chno_tbl * tbl = calloc(sizeof(struct chno_tbl), 1);

    tbl->buckets = 16;
    tbl->nodes   = calloc(sizeof(struct chno_tbln *), 16);
    return tbl;
}

static int
chno_tbl_for_each(struct chno_tbl * tbl, chno_tbl_iter_cb cb, void * args) {
    int i;

    if (tbl == NULL) {
        return -1;
    }

    for (i = 0; i < tbl->buckets; i++) {
        struct chno_tbln * node = tbl->nodes[i];

        while (node) {
            int                res  = 0;
            struct chno_tbln * next = node->next;

            if ((res = cb(node->key, node->val, args))) {
                return res;
            }

            node = next;
        }
    }

    return 0;
}

static void
chno_tbl_destroy(struct chno_tbl * tbl) {
    int i;

    if (tbl == NULL) {
        return;
    }

    for (i = 0; i < tbl->buckets; i++) {
        struct chno_tbln * node = tbl->nodes[i];

        while (node) {
            struct chno_tbln * next = node->next;

            zt_free(node->key);
            zt_free(node);
            node = next;
        }
    }

    zt_free(tbl->nodes);
    zt_free(tbl);
}

static void *
chno_tbl_get(struct chno_tbl * tbl, const char * key) {
    struct chno_tbln * val;
    unsigned int       ki;

    if (key == NULL || tbl == NULL) {
        return NULL;
    }

    ki = _hash(key);

    if ((val = tbl->nodes[ki]) == NULL) {
        return NULL;
    }

    while (val != NULL) {
        if (!strcmp(val->key, key)) {
            return val->val;
        }
        val = val->next;
    }

    return NULL;
}

static void
chno_tbl_del(struct chno_tbl * tbl, const char * key) {
    struct chno_tbln * val;
    struct chno_tbln * prev;
    unsigned int       ki;

    if (key == NULL || tbl == NULL) {
        return;
    }

    ki = _hash(key);

    if ((val = tbl->nodes[ki]) == NULL) {
        return;
    }

    prev = NULL;

    while (val != NULL) {
        if (val->key && !strcmp(val->key, key)) {
            if (prev) {
                prev->next = val->next;
            } else {
                tbl->nodes[ki] = val->next;
            }
            zt_free(val->key);
            zt_free(val);
            return;
        }
        prev = val;
        val  = val->next;
    }

    return;
}

static int
chno_tbl_add(struct chno_tbl * tbl, const char * key, void * val) {
    struct chno_tbln * f;
    struct chno_tbln * n = calloc(sizeof(struct chno_tbln), 1);
    unsigned int       ki;

    if (key == NULL || tbl == NULL) {
        return -1;
    }

    ki     = _hash(key);
    n->key = strdup(key);
    n->val = val;

    if ((f = tbl->nodes[ki]) == NULL) {
        tbl->nodes[ki] = n;
        return 0;
    }

    n->next        = tbl->nodes[ki];
    tbl->nodes[ki] = n;
    tbl->count++;
    return 0;
}


chno_t *
chno_raw_new(void * data, size_t len) {
    chno_t * mbr = NULL;

    if (!(mbr = chno_new(M_TYPE_RAW))) {
        return NULL;
    }

    if (!(M_RAW(mbr) = calloc(sizeof(chno_raw_t), 1))) {
        chno_free(mbr);
        return NULL;
    }

    if (data != NULL && len > 0) {
        M_RAW_DATA(mbr) = calloc(len, 1);
        M_RAW_LEN(mbr)  = len;

        memcpy(M_RAW_DATA(mbr), data, len);
    }

    return mbr;
}

chno_t *
chno_array_new(void) {
    chno_t * mbr = chno_new(M_TYPE_ARRAY);

    if (mbr == NULL) {
        return NULL;
    }

    M_ARR(mbr) = zt_ptr_array_init(NULL, NULL);

    return mbr;
}

chno_t *
chno_map_new(void) {
    chno_t * mbr = chno_new(M_TYPE_MAP);

    if (mbr == NULL) {
        return NULL;
    }

    M_MAP(mbr) = chno_tbl_new();

    return mbr;
}

chno_t *
chno_string_new(const char * str) {
    chno_t * mbr;

    if (str == NULL) {
        return NULL;
    }

    if (!(mbr = chno_new(M_TYPE_STRING))) {
        return NULL;
    }

    M_STR(mbr) = strdup(str);
    return mbr;
}

int
chno_array_add(chno_t * dst, chno_t * src) {
    chno_array_t * dst_arr;
    int            err = 0;

    if (dst == NULL || src == NULL) {
        return -1;
    }

    if (!(dst_arr = chno_array(dst, &err, NULL))) {
        return -1;
    }

    dst->count += 1;

    return zt_ptr_array_add(dst_arr, src);
}

chno_t *
chno_map_del(chno_t * m, const char * k) {
    chno_t     * found = NULL;
    chno_map_t * map   = NULL;
    int          err   = 0;

    if (m == NULL || k == NULL) {
        return NULL;
    }

    if (!(map = chno_map(m, &err, NULL))) {
        return NULL;
    }

    if ((found = chno_map_get(m, k)) == NULL) {
        return NULL;
    }

    chno_tbl_del(map, k);

    return found;
}

int
chno_map_replace(chno_t * map, chno_t * new_val, const char * key) {
    chno_t * old_val;


    if (chno_valid_type(map, M_TYPE_MAP) == false) {
        return -1;
    }

    if (new_val == NULL || key == NULL) {
        return -1;
    }

    if (!(old_val = chno_map_del(map, key))) {
        chno_free(old_val);
    }

    return chno_map_add(map, new_val, key);
}

int
chno_map_add(chno_t * dst, chno_t * src, const char * k) {
    chno_t     * found   = NULL;
    chno_map_t * dst_map = NULL;
    int          err     = 0;

    if (dst == NULL) {
        return -1;
    }

    if (!(dst_map = chno_map(dst, &err, NULL))) {
        return -1;
    }


    found = (chno_t*)chno_tbl_get(dst_map, k);

    if (found != NULL) {
        if (found->type != M_TYPE_ARRAY) {
            /* convert a single value to an array */
            chno_t * n = chno_array_new();

            if (n == NULL) {
                return -1;
            }

            chno_array_add(n, found);
            chno_array_add(n, src);

            chno_tbl_del(dst_map, k);
            return chno_tbl_add(dst_map, k, n);
        }

        return chno_array_add(found, src);
    }

    dst->count += 1;
    return chno_tbl_add(dst_map, k, src);
} /* chno_map_add */

int
chno_add(chno_t * dst, chno_t * src, const char * k) {
    if (!dst || !src) {
        return -1;
    }

    switch (dst->type) {
        case M_TYPE_MAP:
            if (k == NULL) {
                return -1;
            }

            return chno_map_add(dst, src, k);
        case M_TYPE_ARRAY:
            return chno_array_add(dst, src);
        default:
            break;
    }

    return -1;
}

uint32_t
chno_len(chno_t * m) {
    int            err = 0;
    char         * str;
    chno_array_t * arr;
    chno_map_t   * map;
    chno_raw_t   * raw;

    if (m == NULL) {
        return 0;
    }

    switch (chno_type(m)) {
        case M_TYPE_ARRAY:
            if (!(arr = chno_array(m, &err, NULL))) {
                return 0;
            }

            return zt_ptr_array_length(arr);
        case M_TYPE_MAP:
            if (!(map = chno_map(m, &err, NULL))) {
                return 0;
            }

            return map->count;
        case M_TYPE_STRING:
            if (!(str = chno_string(m, &err, NULL))) {
                return 0;
            }

            return (uint32_t)strlen(str);
        case M_TYPE_RAW:
            if (!(raw = chno_raw(m, &err, NULL))) {
                return 0;
            }
            return (uint32_t)raw->len;
        default:
            break;
    } /* switch */

    return 0;
}     /* chno_len */

static int
_map_iter(const char * key, void * val, void * arg) {
    void      ** args   = (void**)arg;
    void       * cb_arg = args[1];
    chno_iter_cb cb     = (chno_iter_cb)args[0];

    return cb((const char*)key, (chno_t*)val, cb_arg);
}

int
chno_for_each(chno_t * m, chno_iter_cb cb, void * arg) {
    int            err = 0;
    chno_array_t * arr;
    chno_map_t   * map;

    if (m == NULL) {
        return -1;
    }

    switch (chno_type(m)) {
        case M_TYPE_MAP:
            if (!(map = chno_map(m, &err, NULL))) {
                return -1;
            }

            {
                void * cb_args[] = { cb, arg };
                return chno_tbl_for_each(map, _map_iter, cb_args);
            }
            break;
        case M_TYPE_ARRAY:
            if (!(arr = chno_array(m, &err, NULL))) {
                return -1;
            }
            {
                uint32_t len;
                uint32_t i;

                len = chno_len(m);

                for (i = 0; i < len; i++) {
                    int res;
                    if ((res = cb(NULL, chno_array_get(m, i), arg))) {
                        return res;
                    }
                }
            }
            break;
        default:
            return -1;
    } /* switch */

    return 0;
}     /* chno_for_each */

int
chno_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    if (mbr == NULL) {
        return -1;
    }

    switch (mbr->type) {
        case M_TYPE_STRING:
            return chno_string_pack(mbr, buf, m_err);
        case M_TYPE_INT32:
            return chno_int32_pack(mbr, buf, m_err);
        case M_TYPE_UINT32:
            return chno_uint32_pack(mbr, buf, m_err);
        case M_TYPE_INT16:
            return chno_int16_pack(mbr, buf, m_err);
        case M_TYPE_UINT16:
            return chno_uint16_pack(mbr, buf, m_err);
        case M_TYPE_INT8:
            return chno_int8_pack(mbr, buf, m_err);
        case M_TYPE_UINT8:
            return chno_uint8_pack(mbr, buf, m_err);
        case M_TYPE_INT64:
            return chno_int64_pack(mbr, buf, m_err);
        case M_TYPE_UINT64:
            return chno_uint64_pack(mbr, buf, m_err);
        case M_TYPE_MAP:
            return chno_map_pack(mbr, buf, m_err);
        case M_TYPE_ARRAY:
            return chno_array_pack(mbr, buf, m_err);
        case M_TYPE_RAW:
            return chno_raw_pack(mbr, buf, m_err);
        default:
            M_MKERR(m_err, "invalid type");
            return -1;
    }     /* switch */

    return -1;
}

int
chno_pack_buffer(chno_t * mbr, void ** obuf, size_t * olen, char m_err[]) {
    chno_buffer_t * tmp = NULL;
    void          * buf = NULL;
    size_t          len = 0;

    if (mbr == NULL) {
        M_MKERR(m_err, "mbr == NULL");
        return -1;
    }

    if (!(tmp = chno_buffer_new())) {
        return -1;
    }

    *obuf = NULL;

    if (chno_pack(mbr, tmp, m_err) < 0) {
        chno_buffer_free(tmp);
        return -1;
    }

    if ((len = chno_buffer_length(tmp)) == 0) {
        chno_buffer_free(tmp);
        M_MKERR(m_err, "len = 0");
        return -1;
    }


    if (!(buf = zt_malloc(char, len))) {
        M_MKERR(m_err, "malloc()");
        chno_buffer_free(tmp);
        return -1;
    }

    if (chno_buffer_remove(tmp, buf, len) < 0) {
        M_MKERR(m_err, "chno_buffer_remove");
        chno_buffer_free(tmp);
        *obuf = NULL;
        return -1;
    }

    *obuf = buf;
    *olen = len;

    chno_buffer_free(tmp);
    return 0;
} /* chno_pack_buffer */

int
chno_pack_buffer_compress(chno_t * mbr, void ** obuf, size_t * olen, char m_err[]) {
    chno_buffer_t * tmp = NULL;
    void          * buf = NULL;
    size_t          len = 0;

    if (mbr == NULL) {
        M_MKERR(m_err, "mbr == NULL");
        return -1;
    }

    if (!(tmp = chno_buffer_new())) {
        return -1;
    }

    if (chno_pack_compress(mbr, tmp, m_err) < 0) {
        chno_buffer_free(tmp);
        return -1;
    }

    if ((len = chno_buffer_length(tmp)) == 0) {
        chno_buffer_free(tmp);
        M_MKERR(m_err, "len = 0");
        return -1;
    }


    if (!(buf = zt_malloc(char, len))) {
        M_MKERR(m_err, "malloc()");
        chno_buffer_free(tmp);
        return -1;
    }

    if (chno_buffer_remove(tmp, buf, len) < 0) {
        M_MKERR(m_err, "chno_buffer_remove");
        chno_buffer_free(tmp);
        return -1;
    }

    *obuf = buf;
    *olen = len;

    chno_buffer_free(tmp);
    return 0;
} /* chno_pack_buffer_compress */

int
chno_pack_compress(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    unsigned char   buffer[2048] = { 0 };
    chno_buffer_t * packed       = NULL;
    z_stream        stream;

    if (mbr == NULL || buf == NULL) {
        M_MKERR(m_err, "args");
        return -1;
    }

    memset(&stream, 0, sizeof(stream));

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        M_MKERR(m_err, "deflateInit()");
        return -1;
    }

    if (!(packed = chno_buffer_new())) {
        M_MKERR(m_err, "chno_buffer_new()");
        return -1;
    }

    if (chno_pack(mbr, packed, m_err) < 0) {
        chno_buffer_free(packed);
        return -1;
    }

    deflateReset(&stream);

    stream.next_in  = chno_buffer_get(packed);
    stream.avail_in = (unsigned int)chno_buffer_length(packed);

    do {
        stream.next_out  = buffer;
        stream.avail_out = sizeof(buffer);

        if (deflate(&stream, Z_FULL_FLUSH) != Z_OK) {
            M_MKERR(m_err, "deflate()");
            deflateEnd(&stream);
            chno_buffer_free(packed);
            return -1;
        }

        chno_buffer_add(buf, buffer, sizeof(buffer) - stream.avail_out);
    } while (stream.avail_out == 0);

    deflateEnd(&stream);
    chno_buffer_free(packed);
    return 0;
} /* chno_pack_compress */

static int
_array_pack_iter(const char * k UNUSED, chno_t * m, void * arg) {
    void         ** args  = (void**)arg;
    char          * m_err = (char*)args[1];
    chno_buffer_t * buf   = (chno_buffer_t*)args[0];

    return chno_pack(m, buf, m_err);
}

int
chno_array_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    chno_packed_hdr_t hdr    = M_TYPE_ARRAY;
    const void      * args[] = { buf, m_err };

    if (chno_valid_type(mbr, M_TYPE_ARRAY) == false) {
        M_MKERR(m_err, "type != M_TYPE_ARRAY");
        return -1;
    }

    chno_buffer_add(buf, &hdr, sizeof(hdr));
    chno_buffer_add(buf, &mbr->count, sizeof(uint32_t));

    return chno_for_each(mbr, _array_pack_iter, (void*)args);
}

static int
_map_pack_iter(const char * k, chno_t * m, void * arg) {
    void         ** args  = (void**)arg;
    chno_buffer_t * buf   = (chno_buffer_t*)args[0];
    char          * m_err = (char*)args[1];

    if (k == NULL) {
        return -1;
    }

    chno_buffer_add(buf, (void *)k, strlen(k) + 1);

    return chno_pack(m, buf, m_err);
}

int
chno_map_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    chno_packed_hdr_t hdr    = M_TYPE_MAP;
    const void      * args[] = { buf, m_err };

    if (chno_valid_type(mbr, M_TYPE_MAP) == false) {
        M_MKERR(m_err, "type != M_TYPE_MAP");
        return -1;
    }

    chno_buffer_add(buf, &hdr, sizeof(chno_packed_hdr_t));
    chno_buffer_add(buf, &mbr->count, sizeof(uint32_t));

    return chno_for_each(mbr, _map_pack_iter, (void*)args);
}

int
chno_string_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    chno_packed_hdr_t hdr = M_TYPE_STRING;
    char            * str;
    int               err = 0;

    if (buf == NULL) {
        return -1;
    }

    if (!(str = chno_string(mbr, &err, m_err))) {
        return -1;
    }

    chno_buffer_add(buf, &hdr, sizeof(hdr));
    chno_buffer_add(buf, str, strlen(str) + 1);
    return 0;
}

int
chno_raw_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) {
    size_t            len     = 0;
    void            * data    = NULL;
    chno_packed_hdr_t hdr     = M_TYPE_RAW;
    uint8_t           word_sz = __WORDSIZE;

    if (chno_valid_type(mbr, M_TYPE_RAW) == false || buf == NULL) {
        return -1;
    }

    if ((len = M_RAW_LEN(mbr)) == 0) {
        return -1;
    }

    if (!(data = M_RAW_DATA(mbr))) {
        return -1;
    }

    /* based on the length of the data, we can create a wordsize that can
     * be better for things like network IO. Since a size_t is system dependent,
     * it can range from 4 to 8 bytes in most cases.
     *
     * Using the wordsize byte, we can easily create smaller payloads based
     * on the actual length of the data (which will be upcasted to a size_t when
     * unpacking) */

    if (len <= UINT8_MAX) {
        word_sz = 8;
    } else if (len <= UINT16_MAX) {
        word_sz = 16;
    } else if (len <= UINT32_MAX) {
        word_sz = 32;
    } else if (len <= UINT64_MAX) {
        word_sz = 64;
    } else {
        return -1;
    }

    chno_buffer_add(buf, &hdr, sizeof(chno_packed_hdr_t));
    chno_buffer_add(buf, &word_sz, sizeof(uint8_t));
    chno_buffer_add(buf, &len, (word_sz >> 3));
    chno_buffer_add(buf, data, len);
    return 0;
}

chno_t *
chno_map_unpack(chno_buffer_t * buf, char m_err[]) {
    chno_t * mbr;
    uint32_t count;
    uint32_t i;

    if (M_BUFCHK(buf, sizeof(uint32_t))) {
        M_BUFERR(m_err);
        return NULL;
    }

    if (!(mbr = chno_map_new())) {
        return NULL;
    }

    chno_buffer_remove(buf, &count, sizeof(uint32_t));

    for (i = 0; i < count; i++) {
        const char * key;
        chno_t     * m;

        if (M_BUFCHK(buf, 2)) {
            M_BUFERR(m_err);
            return NULL;
        }

        if (!(key = strdup((char*)chno_buffer_get(buf)))) {
            chno_free(mbr);
            return NULL;
        }

        chno_buffer_drain(buf, strlen(key) + 1);

        if (key == NULL) {
            M_MKERR(m_err, "key err");
            return NULL;
        }

        if (!(m = chno_unpack(buf, m_err))) {
            break;
        }

        chno_add(mbr, m, key);
        zt_free(key);
    }

    return mbr;
}     /* chno_map_unpack */

chno_t *
chno_array_unpack(chno_buffer_t * buf, char m_err[]) {
    chno_t * mbr;
    uint32_t count;
    uint32_t i;

    if (M_BUFCHK(buf, sizeof(int32_t))) {
        M_BUFERR(m_err);
        return NULL;
    }

    if (!(mbr = chno_array_new())) {
        return NULL;
    }

    chno_buffer_remove(buf, &count, sizeof(int32_t));

    for (i = 0; i < count; i++) {
        chno_t * m;

        if (!(m = chno_unpack(buf, m_err))) {
            break;
        }

        chno_array_add(mbr, m);
    }

    return mbr;
}

chno_t *
chno_raw_unpack(chno_buffer_t * buf, char m_err[]) {
    void   * data    = NULL;
    size_t   len     = 0;
    uint8_t  word_sz = 0;
    uint64_t sz_64   = 0;
    chno_t * m       = NULL;

    if (buf == NULL) {
        return NULL;
    }

    if (M_BUFCHK(buf, sizeof(uint8_t))) {
        M_BUFERR(m_err);
        return NULL;
    }

    chno_buffer_remove(buf, &word_sz, sizeof(uint8_t));

    switch (word_sz) {
        case 8:
            if (M_BUFCHK(buf, sizeof(uint8_t))) {
                M_BUFERR(m_err);
                return NULL;
            }

            chno_buffer_remove(buf, &len, sizeof(uint8_t));
            break;
        case 16:
            if (M_BUFCHK(buf, sizeof(uint16_t))) {
                M_BUFERR(m_err);
                return NULL;
            }

            chno_buffer_remove(buf, &len, sizeof(uint16_t));
            break;
        case 32:
            if (M_BUFCHK(buf, sizeof(uint32_t))) {
                M_BUFERR(m_err);
                return NULL;
            }

            chno_buffer_remove(buf, &len, sizeof(uint32_t));
            break;
        case 64:
            if (M_BUFCHK(buf, sizeof(uint64_t))) {
                M_BUFERR(m_err);
                return NULL;
            }

            sz_64 = *((uint64_t *)chno_buffer_get(buf));

            /* do this check to make sure that the data wouldn't overflow a
             * size_t (think 64b > 32b). If an overflow is detected it will
             * error out. This is in order to use the numerous functions that
             * rely on size_t
             */

            if (sz_64 > SIZE_MAX) {
                M_MKERR(m_err, "size_t would overflow!");
                return NULL;
            }

            chno_buffer_remove(buf, &len, sizeof(uint64_t));
            break;
        default:
            M_MKERR(m_err, "Invalid word size\n");
            return NULL;
    } /* switch */

    if (M_BUFCHK(buf, len)) {
        M_BUFERR(m_err);
        return NULL;
    }

    if (!(data = calloc(len, 1))) {
        return NULL;
    }

    chno_buffer_remove(buf, data, len);

    if (!(m = chno_raw_new(data, len))) {
        zt_free(data);
        return NULL;
    }

    zt_free(data);

    return m;
} /* chno_raw_unpack */

chno_t *
chno_string_unpack(chno_buffer_t * buf, char m_err[]) {
    const char * str;
    void       * data;
    void       * zptr;
    size_t       slen;
    chno_t     * m;

    if (buf == NULL) {
        return NULL;
    }

    if (M_BUFCHK(buf, 1)) {
        M_BUFERR(m_err);
        return NULL;
    }

    data = chno_buffer_get(buf);

    if (!(zptr = memchr(data, '\0', chno_buffer_length(buf)))) {
        M_MKERR(m_err, "str err");
        return NULL;
    }

    slen = (size_t)(zptr - data) + 1;
    str  = alloca(slen);

    chno_buffer_remove(buf, (void *)str, slen);

    if (!(m = chno_string_new(str))) {
        chno_free(m);
        return NULL;
    }

    return m;
}

chno_t *
chno_unpack(chno_buffer_t * buf, char m_err[]) {
    chno_packed_hdr_t hdr = M_TYPE_START;

    chno_buffer_remove(buf, &hdr, sizeof(chno_packed_hdr_t));

    switch (hdr) {
        case M_TYPE_MAP:
            return chno_map_unpack(buf, m_err);
        case M_TYPE_ARRAY:
            return chno_array_unpack(buf, m_err);
        case M_TYPE_STRING:
            return chno_string_unpack(buf, m_err);
        case M_TYPE_INT32:
            return chno_int32_unpack(buf, m_err);
        case M_TYPE_UINT32:
            return chno_uint32_unpack(buf, m_err);
        case M_TYPE_UINT16:
            return chno_uint16_unpack(buf, m_err);
        case M_TYPE_INT16:
            return chno_int16_unpack(buf, m_err);
        case M_TYPE_UINT8:
            return chno_uint8_unpack(buf, m_err);
        case M_TYPE_INT8:
            return chno_int8_unpack(buf, m_err);
        case M_TYPE_INT64:
            return chno_int64_unpack(buf, m_err);
        case M_TYPE_UINT64:
            return chno_uint64_unpack(buf, m_err);
        case M_TYPE_RAW:
            return chno_raw_unpack(buf, m_err);
    }         /* switch */

    return NULL;
}

chno_t *
chno_unpack_buffer(void * data, size_t len, char m_err[]) {
    chno_buffer_t * tmp   = NULL;
    chno_t        * ret_m = NULL;

    if (data == NULL || len == 0) {
        M_MKERR(m_err, "data == NULL || len == 0");
        return NULL;
    }

    tmp = chno_buffer_new();
    chno_buffer_add(tmp, data, len);

    ret_m = chno_unpack(tmp, m_err);
    chno_buffer_free(tmp);
    return ret_m;
}

chno_t *
chno_unpack_buffer_compressed(void * data, size_t len, char m_err[]) {
    chno_buffer_t * tmp   = NULL;
    chno_t        * ret_m = NULL;

    if (data == NULL || len == 0) {
        M_MKERR(m_err, "data == NULL || len == 0");
        return NULL;
    }

    if (!(tmp = chno_buffer_new())) {
        return NULL;
    }

    chno_buffer_add(tmp, data, len);

    ret_m = chno_unpack_compressed(tmp, m_err);
    chno_buffer_free(tmp);
    return ret_m;
}

chno_t *
chno_unpack_compressed(chno_buffer_t * buf, char m_err[]) {
    z_stream        stream;
    chno_buffer_t * tmp          = NULL;
    chno_t        * unpacked_m   = NULL;
    bool            done         = false;
    unsigned char   buffer[2048] = { 0 };

    if (buf == NULL) {
        M_MKERR(m_err, "buf == NULL");
        return NULL;
    }

    memset(&stream, 0, sizeof(z_stream));

    if (inflateInit(&stream) != Z_OK) {
        M_MKERR(m_err, "inflateInit()");
        return NULL;
    }

    if (!(tmp = chno_buffer_new())) {
        M_MKERR(m_err, "chno_buffer_new()");
        return NULL;
    }

    inflateReset(&stream);

    stream.next_in  = chno_buffer_get(buf);
    stream.avail_in = (unsigned int)chno_buffer_length(buf);

    do {
        stream.next_out  = buffer;
        stream.avail_out = sizeof(buffer);

        switch (inflate(&stream, Z_FULL_FLUSH)) {
            case Z_OK:
                chno_buffer_add(tmp, buffer, sizeof(buffer) - stream.avail_out);
                break;
            case Z_BUF_ERROR:
                done = true;
                break;
            default:
                M_MKERR(m_err, "inflate()");
                inflateEnd(&stream);
                chno_buffer_free(tmp);
                return NULL;
        }
    } while (done == false);

    unpacked_m = chno_unpack(tmp, m_err);
    chno_buffer_drain(buf, stream.total_in);
    chno_buffer_free(tmp);
    inflateEnd(&stream);

    return unpacked_m;
}         /* chno_unpack_compressed */

chno_t *
chno_from_raw(chno_t * raw) {
    chno_t        * m      = NULL;
    chno_raw_t    * rawp   = NULL;
    chno_buffer_t * packed = NULL;
    int             err    = 0;
    char            m_err[M_ERROR_SZ];

    if (raw == NULL) {
        return NULL;
    }

    if (!(rawp = chno_raw(raw, &err, m_err))) {
        return NULL;
    }

    if (!(packed = chno_buffer_new())) {
        return NULL;
    }

    chno_buffer_add(packed, rawp->data, rawp->len);

    m = chno_unpack(packed, m_err);

    chno_buffer_free(packed);
    return m;
}

chno_t *
chno_copy(chno_t * in_m) {
    /* FIXME HACK: this function "copies" a chno_t into a new one, but
     * this packs it, then unpacks it. Should just create a real copy.
     */
    chno_t        * out_m = NULL;
    chno_buffer_t * packed;
    char            m_err[M_ERROR_SZ];

    if (in_m == NULL) {
        return NULL;
    }

    if (!(packed = chno_buffer_new())) {
        return NULL;
    }

    if (chno_pack(in_m, packed, m_err) < 0) {
        chno_buffer_free(packed);
        return NULL;
    }

    out_m = chno_unpack(packed, m_err);
    chno_buffer_free(packed);

    return out_m;
}

chno_t *
chno_map_get(chno_t * m, const char * key) {
    chno_map_t * map = NULL;
    int          err = 0;

    if (m == NULL || key == NULL) {
        return NULL;
    }

    if (!(map = chno_map(m, &err, NULL))) {
        return NULL;
    }

    return (chno_t *)chno_tbl_get(map, key);
}

chno_t *
chno_array_get(chno_t * m, int32_t idx) {
    chno_array_t * arr = NULL;
    int            err = 0;

    if (m == NULL || idx < 0) {
        return NULL;
    }

    if (!(arr = chno_array(m, &err, NULL))) {
        return NULL;
    }

    return (chno_t *)zt_ptr_array_get_idx(arr, idx);
}

chno_t *
chno_array_del(chno_t * m, uint32_t idx) {
    uint32_t i       = 0;
    chno_t * new_arr = NULL;

    if (chno_valid_type(m, M_TYPE_ARRAY) == false) {
        return NULL;
    }

    if (!(new_arr = chno_array_new())) {
        return NULL;
    }

    for (i = 0; i < chno_len(m); i++) {
        if (i == idx) {
            continue;
        }

        chno_add(new_arr, chno_copy(chno_array_get(m, i)), NULL);
    }

    chno_free(m);

    return new_arr;
}

#if 0
inline bool
chno_valid_type(chno_t * mbr, chno_type_t type) {
    if (mbr->type != type) {
        return false;
    }

    return true;
}

#endif

inline chno_type_t
chno_type(chno_t * m) {
    if (m == NULL) {
        return 0;
    }

    return m->type;
}

static int
_map_free_iter(const char * k UNUSED, chno_t * m, void * arg UNUSED) {
    chno_free(m);
    return 0;
}

static int
_array_free_iter(const char * k UNUSED, chno_t * m, void * arg UNUSED) {
    chno_free(m);
    return 0;
}

void
chno_map_free(chno_t * m) {
    chno_map_t * map = NULL;
    int          err = 0;

    if (m == NULL) {
        return;
    }

    if (!(map = chno_map(m, &err, NULL))) {
        return;
    }

    chno_for_each(m, _map_free_iter, NULL);

    chno_tbl_destroy(map);
    zt_free(m);
}

void
chno_array_free(chno_t * m) {
    chno_array_t * arr = NULL;
    int            err = 0;

    if (m == NULL) {
        return;
    }

    if (!(arr = chno_array(m, &err, NULL))) {
        return;
    }

    chno_for_each(m, _array_free_iter, NULL);
    zt_ptr_array_free(arr, 0);
    zt_free(m);
}

void
chno_str_free(chno_t * m) {
    char * str = NULL;
    int    err = 0;

    if (m == NULL) {
        return;
    }

    str = chno_string(m, &err, NULL);

    zt_free(str);
    zt_free(m);
}

void
chno_raw_free(chno_t * m) {
    chno_raw_t * raw      = NULL;
    void       * raw_data = NULL;
    int          err      = 0;

    if (m == NULL) {
        return;
    }

    if ((raw = chno_raw(m, &err, NULL)) != NULL) {
        raw_data = raw->data;
    }

    zt_free(raw_data);
    zt_free(raw);
    zt_free(m);
}

void
chno_free(chno_t * m) {
    if (m == NULL) {
        return;
    }

    switch (chno_type(m)) {
        case M_TYPE_MAP:
            return chno_map_free(m);
        case M_TYPE_ARRAY:
            return chno_array_free(m);
        case M_TYPE_STRING:
            return chno_str_free(m);
        case M_TYPE_RAW:
            return chno_raw_free(m);
        default:
            zt_free(m);
            break;
    }
}

void *
chno_data(chno_t * m) {
    chno_raw_t * raw = NULL;
    int          err = 0;

    if (m == NULL) {
        return NULL;
    }

    switch (chno_type(m)) {
        case M_TYPE_RAW:
            if (!(raw = chno_raw(m, &err, NULL))) {
                return NULL;
            }

            if (raw->len == 0) {
                return NULL;
            }

            return raw->data;
        default:
            break;
    }

    return NULL;
}

int
chno_set_arg(chno_t * m, void * arg) {
    if (m == NULL) {
        return -1;
    }

    m->arg = arg;
    return 0;
}

void *
chno_get_arg(chno_t * m) {
    if (m == NULL) {
        return NULL;
    }

    return m->arg;
}

