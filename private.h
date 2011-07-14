#ifndef __CHNO_PRIVATE__
#define __CHNO_PRIVATE__
#include <zt.h>
#include <stdint.h>

#define __NATIVE_WORD_BYTE_SZ (__WORDSIZE / 8)

#define CHNO_M_GENERATE_INTTYPE_FUN(vname, mtype, type)                            \
    chno_t * chno_ ## vname ## _new(type d) {                                      \
        chno_t * mbr = chno_new(mtype);                                            \
                                                                                   \
        if (mbr == NULL) {                                                         \
            return NULL;                                                           \
        }                                                                          \
                                                                                   \
        mbr->data.vname = d;                                                       \
        return mbr;                                                                \
    }                                                                              \
                                                                                   \
    int chno_ ## vname ## _pack(chno_t * mbr, chno_buffer_t * buf, char m_err[]) { \
        chno_packed_hdr_t hdr = mtype;                                             \
        type              n;                                                       \
                                                                                   \
        if (buf == NULL) {                                                         \
            return -1;                                                             \
        }                                                                          \
                                                                                   \
        if (chno_valid_type(mbr, mtype) == false) {                                \
            M_MKERR(m_err, "bad mbr\n");                                           \
            return -1;                                                             \
        }                                                                          \
                                                                                   \
        n = mbr->data.vname;                                                       \
        chno_buffer_add(buf, &hdr, sizeof(chno_packed_hdr_t));                     \
        chno_buffer_add(buf, &n, sizeof(type));                                    \
        return 0;                                                                  \
    }                                                                              \
                                                                                   \
    chno_t * chno_ ## vname ## _unpack(chno_buffer_t * buf, char m_err[]) {        \
        type d;                                                                    \
                                                                                   \
        if (buf == NULL) {                                                         \
            return NULL;                                                           \
        }                                                                          \
                                                                                   \
        if (M_BUFCHK(buf, sizeof(type))) {                                         \
            M_BUFERR(m_err);                                                       \
            return NULL;                                                           \
        }                                                                          \
                                                                                   \
        chno_buffer_remove(buf, &d, sizeof(type));                                 \
        return chno_ ## vname ## _new(d);                                          \
    }


#define CHNO_GENERATE_VAL_FUN(_vname, _mtype, _type, _errval)             \
    inline _type chno_ ## _vname(chno_t * mbr, int * err, char m_err[]) { \
                                                                          \
        if (chno_valid_type(mbr, _mtype) == false) {                      \
            M_MKERR(m_err, "member mismatch");                            \
            *err = 1;                                                     \
            return _errval;                                               \
        }                                                                 \
                                                                          \
        *err = 0;                                                         \
        return mbr->data._vname;                                          \
    }


#define CHNO_M_GENERATE                                              \
    CHNO_M_GENERATE_INTTYPE_FUN(uint32, M_TYPE_UINT32, uint32_t)     \
    CHNO_M_GENERATE_INTTYPE_FUN(int32, M_TYPE_INT32, int32_t)        \
    CHNO_M_GENERATE_INTTYPE_FUN(uint16, M_TYPE_UINT16, uint16_t)     \
    CHNO_M_GENERATE_INTTYPE_FUN(int16, M_TYPE_INT16, int16_t)        \
    CHNO_M_GENERATE_INTTYPE_FUN(uint8, M_TYPE_UINT8, uint8_t)        \
    CHNO_M_GENERATE_INTTYPE_FUN(int8, M_TYPE_INT8, int8_t)           \
    CHNO_M_GENERATE_INTTYPE_FUN(uint64, M_TYPE_UINT64, uint64_t)     \
    CHNO_M_GENERATE_INTTYPE_FUN(int64, M_TYPE_INT64, int64_t)        \
                                                                     \
    CHNO_GENERATE_VAL_FUN(uint32, M_TYPE_UINT32, uint32_t, 0)        \
    CHNO_GENERATE_VAL_FUN(int32, M_TYPE_INT32, int32_t, -1)          \
    CHNO_GENERATE_VAL_FUN(uint16, M_TYPE_UINT16, uint16_t, 0)        \
    CHNO_GENERATE_VAL_FUN(int16, M_TYPE_INT16, int16_t, -1)          \
    CHNO_GENERATE_VAL_FUN(uint8, M_TYPE_UINT8, uint8_t, 0)           \
    CHNO_GENERATE_VAL_FUN(int8, M_TYPE_INT8, int8_t, -1)             \
    CHNO_GENERATE_VAL_FUN(uint64, M_TYPE_UINT64, uint64_t, 0)        \
    CHNO_GENERATE_VAL_FUN(int64, M_TYPE_INT64, int64_t, -1)          \
    CHNO_GENERATE_VAL_FUN(string, M_TYPE_STRING, char *, NULL)       \
    CHNO_GENERATE_VAL_FUN(raw, M_TYPE_RAW, chno_raw_t *, NULL)       \
    CHNO_GENERATE_VAL_FUN(array, M_TYPE_ARRAY, chno_array_t *, NULL) \
    CHNO_GENERATE_VAL_FUN(map, M_TYPE_MAP, chno_map_t *, NULL)
#endif
