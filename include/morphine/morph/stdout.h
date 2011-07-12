#ifndef __CHNO_MORPH_STDOUT_H__
#define __CHNO_MORPH_STDOUT_H__
/* includes */

BEGIN_C_DECLS

/* module declarations */
void chno_print(chno_t *);
void chno_print_m(chno_t *);
void chno_string_print(chno_t *);
void chno_int32_print(chno_t *);
void chno_uint32_print(chno_t *);
void chno_array_print(chno_t *);
void chno_map_print(chno_t * mbr);
void chno_hexdump(char * d, const size_t len);

END_C_DECLS

#endif

