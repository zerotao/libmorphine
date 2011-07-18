#ifndef __CHNO_MORPH_JSON_H__
#define __CHNO_MORPH_JSON_H__
/* includes */
#include <json.h>
#include <zt.h>

BEGIN_C_DECLS

/* module declarations */

chno_t      * chno_from_jarr(json_object * jobj);
chno_t      * chno_from_jobj(json_object * jobj);
chno_t      * chno_from_jtype(json_object * jobj);
chno_t      * chno_from_json(const char * jsonbuf);

json_object * chno_to_json(chno_t *);

END_C_DECLS

#endif

