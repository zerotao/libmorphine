#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <alloca.h>
#include <inttypes.h>
#include <zt.h>

#include "morphine/util.h"
#include "morphine/base.h"
#include "morphine/morph/stdout.h"

void
chno_string_print(chno_t * mbr) {
    printf(" \"%s\"", M_STR(mbr));
}

void
chno_int32_print(chno_t * mbr) {
    printf(" %d", M_INT32(mbr));
}

void
chno_uint32_print(chno_t * mbr) {
    printf(" %u", M_UINT32(mbr));
}

void
chno_uint16_print(chno_t * mbr) {
    printf(" %u", M_UINT16(mbr));
}

void
chno_int16_print(chno_t * mbr) {
    printf(" %d", M_INT16(mbr));
}

void
chno_uint8_print(chno_t * mbr) {
    printf(" %u", M_UINT8(mbr));
}

void
chno_int8_print(chno_t * mbr) {
    printf(" %u", M_INT8(mbr));
}

void
chno_raw_print(chno_t * mbr) {
    chno_hexdump(M_RAW_DATA(mbr), M_RAW_LEN(mbr));
}

void
chno_int64_print(chno_t * mbr) {
    printf(" %" PRId64, M_INT64(mbr));
}

void
chno_uint64_print(chno_t * mbr) {
    printf(" %" PRIu64, M_UINT64(mbr));
}

void
chno_print_m(chno_t * mbr) {
    switch (mbr->type) {
        case M_TYPE_MAP:
            return chno_map_print(mbr);
        case M_TYPE_ARRAY:
            return chno_array_print(mbr);
        case M_TYPE_STRING:
            return chno_string_print(mbr);
        case M_TYPE_INT32:
            return chno_int32_print(mbr);
        case M_TYPE_UINT32:
            return chno_uint32_print(mbr);
        case M_TYPE_UINT16:
            return chno_uint16_print(mbr);
        case M_TYPE_INT16:
            return chno_int16_print(mbr);
        case M_TYPE_UINT8:
            return chno_uint8_print(mbr);
        case M_TYPE_INT8:
            return chno_int8_print(mbr);
        case M_TYPE_RAW:
            return chno_raw_print(mbr);
        case M_TYPE_UINT64:
            return chno_uint64_print(mbr);
        case M_TYPE_INT64:
            return chno_int64_print(mbr);
        default:
            printf(",");
            return;
    } /* switch */
}

void
chno_print(chno_t * mbr) {
    chno_print_m(mbr);
    printf("\n");
}

void
chno_array_print(chno_t * mbr) {
    int i;

    printf(" [");

    for (i = 0; i < M_ARR(mbr)->count; i++) {
        chno_t * array_mbr;
        array_mbr = (chno_t *)zt_ptr_array_get_idx(M_ARR(mbr), i);
        chno_print_m(array_mbr);

        if (i + 1 < M_ARR(mbr)->count) {
            printf(", ");
        }
    }
    printf(" ]");
}

static int
chno_map_iter_print(const char * key, chno_t * val, void * args UNUSED) {
    printf(" \"%s\":", key);
    chno_print_m(val);

    return 0;
}

void
chno_map_print(chno_t * mbr) {
    printf(" {");
    chno_for_each(mbr, chno_map_iter_print, NULL);
    printf(" }");
}

void
chno_hexdump(char * data, const size_t len) {
    int          line_buf_off;
    char       * line_buf = NULL;
    int          spaces_left;
    char       * spaces   = "                                          ";
    unsigned int i;

    line_buf = alloca(len + 30);

    memset(line_buf, 0, len + 30);

    line_buf_off = 0;
    spaces_left  = 41;

    printf("      ");

    for (i = 0; i < len; i++) {
        if (i && !(i % 16)) {
            printf(" %s\n     ", line_buf);
            memset(line_buf, 0, len + 3);
            line_buf_off = -1;
            spaces_left  = 41;
        }

        if (line_buf_off < 0) {
            line_buf[++line_buf_off] = 'f';
        }

        if (line_buf_off >= 0) {
            if (isprint(data[i]) && !isspace(data[i])) {
                line_buf[line_buf_off] = data[i];
            } else {
                line_buf[line_buf_off] = '.';
            }
        }

        if (i && !(i % 2)) {
            spaces_left -= 1;
            printf(" ");
        }

        printf("%2.2X", (uint8_t)data[i]);
        line_buf_off++;

        spaces_left -= 2;
    }

    printf("%.*s%s\n", spaces_left,
           spaces, line_buf);
} /* chno_hexdump */

