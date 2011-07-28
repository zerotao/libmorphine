#ifndef __CHNO_BASE_H__
#define __CHNO_BASE_H__

/* includes */
#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
#include <sys/queue.h>
#include <zt.h>

#ifndef __WORDSIZE
#error Couldnt find native wordsize.
#endif

#if __WORDSIZE != 32 && __WORDSIZE != 64
#error Invalid native wordsize
#endif

/* BEGIN_C_DECLS */

typedef struct mm       chno_t;
typedef struct chno_raw chno_raw_t;

typedef zt_ptr_array    chno_array_t;
typedef uint8_t         chno_packed_hdr_t;

struct chno_tbl;
typedef struct chno_tbl chno_map_t;

typedef enum {
    M_TYPE_START = 0,
    M_TYPE_ARRAY,
    M_TYPE_MAP,
    M_TYPE_STRING,
    M_TYPE_INT32,
    M_TYPE_UINT32,
    M_TYPE_RAW,
    M_TYPE_UINT16,
    M_TYPE_INT16,
    M_TYPE_UINT8,
    M_TYPE_INT8,
    M_TYPE_INT64,
    M_TYPE_UINT64,
    M_TYPE_END
} chno_type_t;

typedef union chno_union {
    chno_array_t * array;
    chno_map_t   * map;
    chno_raw_t   * raw;
    char         * string;
    int32_t        int32;
    uint32_t       uint32;
    uint16_t       uint16;
    int16_t        int16;
    uint8_t        uint8;
    int8_t         int8;
    int64_t        int64;
    uint64_t       uint64;
} chno_union_t;


struct chno_tbln {
    char * key;
    void * val;

    struct chno_tbln * next;
};

struct chno_tbl {
    int                 buckets;
    uint32_t            count;
    struct chno_tbln ** nodes;
};

typedef int (*chno_iter_cb)(const char * key, chno_t * val, void * args);

#define M_ERROR_SZ 1024

#define M_STR(d)           ((d)->data.string)
#define M_UINT32(d)        ((d)->data.uint32)
#define M_INT32(d)         ((d)->data.int32)
#define M_UINT16(d)        ((d)->data.uint16)
#define M_INT16(d)         ((d)->data.int16)
#define M_UINT8(d)         ((d)->data.uint8)
#define M_INT8(d)          ((d)->data.int8)
#define M_UINT64(d)        ((d)->data.uint64)
#define M_INT64(d)         ((d)->data.int64)
#define M_MAP(d)           ((d)->data.map)
#define M_ARR(d)           ((d)->data.array)
#define M_RAW(d)           ((d)->data.raw)
#define M_RAW_DATA(d)      ((d)->data.raw->data)
#define M_RAW_LEN(d)       ((d)->data.raw->len)

#define M_MKERR(buf, msg)  do {                                                \
        if (buf != NULL) {                                                     \
            snprintf(buf, M_ERROR_SZ - 1, "%s: %s", __FUNCTION__, msg ? : ""); \
        }                                                                      \
} while (0)

#define M_BUFERR(buf)      M_MKERR(buf, "buf too small")
#define M_BUFCHK(buf, len) (chno_buffer_length(buf) < len)

#define CHNO_M_PROTOTYPE_INTTYPE_FUN(vname, type)                                      \
    chno_t * chno_ ## vname ## _new(type d);                                           \
    int      chno_ ## vname ## _pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]); \
    chno_t * chno_ ## vname ## _unpack(chno_buffer_t * buf, char m_err[]);

#define CHNO_PROTOTYPE_VAL_FUN(_vname, _type) \
    _type chno_ ## _vname(chno_t *, int * err, char m_err[]);


#define CHNO_M_PROTOTYPE                           \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(uint32, uint32_t) \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(int32, int32_t)   \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(uint16, uint16_t) \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(int16, int16_t)   \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(uint8, uint8_t)   \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(int8, int8_t)     \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(int64, int64_t)   \
    CHNO_M_PROTOTYPE_INTTYPE_FUN(uint64, uint64_t) \
                                                   \
    CHNO_PROTOTYPE_VAL_FUN(uint32, uint32_t)       \
    CHNO_PROTOTYPE_VAL_FUN(int32, int32_t)         \
    CHNO_PROTOTYPE_VAL_FUN(uint16, uint16_t)       \
    CHNO_PROTOTYPE_VAL_FUN(int16, int16_t)         \
    CHNO_PROTOTYPE_VAL_FUN(uint8, uint8_t)         \
    CHNO_PROTOTYPE_VAL_FUN(int8, int8_t)           \
    CHNO_PROTOTYPE_VAL_FUN(uint64, uint64_t)       \
    CHNO_PROTOTYPE_VAL_FUN(int64, int64_t)         \
    CHNO_PROTOTYPE_VAL_FUN(string, char *)         \
    CHNO_PROTOTYPE_VAL_FUN(raw, chno_raw_t *)      \
    CHNO_PROTOTYPE_VAL_FUN(array, chno_array_t *)  \
    CHNO_PROTOTYPE_VAL_FUN(map, chno_map_t *)


struct mm {
    int32_t      count;
    chno_type_t  type;
    chno_union_t data;
    void       * arg;
};

struct chno_raw {
    void * data;
    size_t len;
};

/* module declarations */
CHNO_M_PROTOTYPE;

void        chno_free(chno_t * m);
chno_t    * chno_new(chno_type_t type);
chno_t    * chno_array_new(void);
chno_t    * chno_string_new(const char * str);
chno_t    * chno_map_new(void);
chno_t    * chno_raw_new(void * data, size_t len);
chno_t    * chno_from_raw(chno_t *);
chno_t    * chno_copy(chno_t * in);
chno_t    * chno_array_get(chno_t *, int32_t);
chno_t    * chno_map_get(chno_t *, const char *);

chno_t    * chno_map_del(chno_t * map, const char * key);
chno_t    * chno_array_del(chno_t * m, uint32_t idx);
int         chno_map_add(chno_t * dst, chno_t * src, const char * key);
int         chno_map_replace(chno_t * map, chno_t * new_val, const char * key);
int         chno_array_add(chno_t * dst, chno_t * src);
int         chno_add(chno_t * dst, chno_t * src, const char * key);
uint32_t    chno_len(chno_t * m);

chno_t    * chno_unpack(chno_buffer_t *, char m_err[]);
chno_t    * chno_unpack_compressed(chno_buffer_t *, char m_err[]);
chno_t    * chno_unpack_buffer(void * data, size_t len, char m_err[]);
chno_t    * chno_unpack_buffer_compressed(void * data, size_t len, char m_err[]);
chno_t    * chno_string_unpack(chno_buffer_t * buf, char m_err[]);
chno_t    * chno_raw_unpack(chno_buffer_t * buf, char m_err[]);
chno_t    * chno_array_unpack(chno_buffer_t *, char m_err[]);
chno_t    * chno_map_unpack(chno_buffer_t *, char m_err[]);

int         chno_pack(chno_t *, chno_buffer_t *, char m_err[]);
int         chno_pack_buffer(chno_t *, void **, size_t *, char m_err[]);
int         chno_pack_buffer_compress(chno_t *, void **, size_t *, char m_err[]);
int         chno_pack_compress(chno_t *, chno_buffer_t *, char m_err[]);
int         chno_map_pack(chno_t *, chno_buffer_t *, char m_err[]);
int         chno_raw_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]);
int         chno_string_pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]);
int         chno_array_pack(chno_t *, chno_buffer_t *, char m_err[]);
int         chno_for_each(chno_t *, chno_iter_cb cb, void *);

chno_type_t chno_type(chno_t *);
/* bool        chno_valid_type(chno_t *, chno_type_t); */
#define chno_valid_type(mbr, _type) ((mbr->type == _type) ? true : false)

int    chno_set_arg(chno_t * m, void * arg);
void * chno_get_arg(chno_t * m);
void * chno_data(chno_t *);
/* END_C_DECLS */

#endif

