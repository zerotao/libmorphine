#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <morphine.h>

static void
pack_and_unpack(chno_t * chno) {
    chno_buffer_t * buf;
    chno_t          * unpacked;

    buf = chno_buffer_new();

    printf("----- Original chno structure\n");
    chno_print(chno);
    printf("-----\n");

    chno_pack(chno, buf, NULL);

    printf("----- Hexdump of the packed chno structure\n");
    chno_hexdump((char *)chno_buffer_get(buf), chno_buffer_length(buf));
    printf("-----\n");

    unpacked = chno_unpack(buf, NULL);

    printf("----- Unpacked chno structure\n");
    chno_print(unpacked);
    printf("-----\n");

    chno_free(unpacked);
    chno_buffer_drain(buf, chno_buffer_length(buf));

    printf("----- Creating a compressed and packed buffer\n");
    chno_pack_compress(chno, buf, NULL);
    printf("-----\n");

    printf("----- Hexdump of the compressed and packed strucutre\n");
    chno_hexdump((char *)chno_buffer_get(buf), chno_buffer_length(buf));
    printf("-----\n");

    unpacked = chno_unpack_compressed(buf, NULL);

    printf("----- Unpacked compressed and packed structure\n");
    chno_print(unpacked);
    printf("----- \n");

    chno_free(unpacked);
    chno_buffer_free(buf);
} /* pack_and_unpack */

void
chno_primitives_example(void) {
    chno_t * uint8  = chno_uint8_new(0);
    chno_t * int8   = chno_int8_new(1);
    chno_t * uint16 = chno_uint16_new(4);
    chno_t * int16  = chno_int16_new(8);
    chno_t * uint32 = chno_uint32_new(16);
    chno_t * int32  = chno_int32_new(32);
    chno_t * uint64 = chno_uint64_new(64);
    chno_t * int64  = chno_int64_new(128);
    chno_t * str    = chno_string_new("foobar");
    chno_t * raw    = chno_raw_new("herpderp\0", 9);
    chno_t * array  = chno_array_new();
    int      err    = 0;

    printf("%" PRIu8 "\n", chno_uint8(uint8, &err, NULL));
    printf("%" PRId8 "\n", chno_int8(int8, &err, NULL));
    printf("%" PRIu16 "\n", chno_uint16(uint16, &err, NULL));
    printf("%" PRId16 "\n", chno_int16(int16, &err, NULL));
    printf("%" PRIu32 "\n", chno_uint32(uint32, &err, NULL));
    printf("%" PRId32 "\n", chno_int32(int32, &err, NULL));
    printf("%" PRIu64 "\n", chno_uint64(uint64, &err, NULL));
    printf("%" PRId64 "\n", chno_int64(int64, &err, NULL));
    printf("%s\n", chno_string(str, &err, NULL));
    printf("raw len = %u data = %.*s\n", chno_len(raw),
           chno_len(raw), (char *)chno_data(raw));

    printf("Adding all primitives to an array\n");
    chno_add(array, uint8, NULL);
    chno_add(array, int8, NULL);
    chno_add(array, uint16, NULL);
    chno_add(array, int16, NULL);
    chno_add(array, uint32, NULL);
    chno_add(array, int32, NULL);
    chno_add(array, uint64, NULL);
    chno_add(array, int64, NULL);
    chno_add(array, str, NULL);
    chno_add(array, raw, NULL);

    chno_print(array);
    pack_and_unpack(array);
} /* chno_primitives_example */

void
chno_map_example(void) {
    /* chno_maps are simple key/value storage structures. Keys are always
     * strings while values can be any chno datatype */
    chno_t * map = chno_map_new();

    /* add some normal key/vals */
    chno_add(map, chno_string_new("val1"), "key1");
    chno_add(map, chno_string_new("val2"), "key2");
    chno_add(map, chno_string_new("val3"), "key3");
    chno_add(map, chno_uint8_new(5), "key4");
    chno_add(map, chno_int32_new(5000), "key5");

    /* add another value to an already existing key, the
     * value will then be converted to an array */
    chno_add(map, chno_int16_new(32), "key1");

    /* display pack and unpack of this chno_map */
    pack_and_unpack(map);

    /* maps can be used just like a hash lookup table */
    chno_t * val_1 = chno_map_get(map, "key1");

    if (chno_type(val_1) != M_TYPE_ARRAY) {
        /* remember, we added two values to key1, thus the value becoming an
         * array of values */
        fprintf(stderr, "SOMETHING WENT HORRIBLY WRONG!\n");
        exit(1);
    }

    printf("val_1 has %u entries\n", chno_len(val_1));

    /* we can now reference each entry in the value by index */
    {
        chno_t * val_1_idx0 = chno_array_get(val_1, 0);
        chno_t * val_1_idx1 = chno_array_get(val_1, 1);
        int      err        = 0;

        printf("val_1_idx0 == %s\n", chno_string(val_1_idx0, &err, NULL));
        printf("val_1_idx1 == %d\n", chno_int16(val_1_idx1, &err, NULL));

        pack_and_unpack(val_1_idx0);
        pack_and_unpack(val_1_idx1);
    }

    chno_free(map);
} /* main */

int
main(int argc, char ** argv) {
    chno_primitives_example();
    chno_map_example();
    return 0;
}

