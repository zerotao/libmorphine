#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <json/json.h>
#include <json/json_object_private.h>

#include "morphine/base.h"
#include "morphine/morph/json.h"

chno_t *
chno_from_jarr(json_object * jobj) {
    chno_t * m;
    int      i;

    m = chno_array_new();

    for (i = 0; i < json_object_array_length(jobj); i++) {
        json_object * jchild;

        jchild = json_object_array_get_idx(jobj, i);
        chno_add(m, chno_from_jtype(jchild), NULL);
    }

    return m;
}

chno_t *
chno_from_jobj(json_object * jobj) {
    json_object_iter jiter;
    chno_t         * m;

    m = chno_map_new();

    json_object_object_foreachC(jobj, jiter) {
        json_object * jchild = jiter.val;

        chno_add(m, chno_from_jtype(jchild), jiter.key);
    }

    return m;
}

chno_t *
chno_from_jtype(json_object * jobj) {
    chno_t * mbr;

    switch (json_object_get_type(jobj)) {
        case json_type_int:
            mbr = chno_int32_new(json_object_get_int(jobj));
            break;
        case json_type_object:
            mbr = chno_from_jobj(jobj);
            break;
        case json_type_string:
            mbr = chno_string_new(json_object_get_string(jobj));
            break;
        case json_type_array:
            mbr = chno_from_jarr(jobj);
            break;
        default:
            break;
    }
    return mbr;
}

chno_t *
chno_from_json(const char * jsonbuf) {
    json_object * jobj;

    if (jsonbuf == NULL) {
        return NULL;
    }

    jobj = json_tokener_parse(jsonbuf);

    if (is_error(jobj)) {
        return NULL;
    }

    return chno_from_jtype(jobj);
}

static int
_chno_to_json_iter(const char * key, chno_t * val, void * arg) {
    json_object * jparent = arg;
    json_object * jchild;

    switch (chno_type(val)) {
        case M_TYPE_MAP:
            jchild = json_object_new_object();
            chno_for_each(val, _chno_to_json_iter, (void*)jchild);
            break;
        case M_TYPE_INT16:
        case M_TYPE_UINT16:
        case M_TYPE_INT8:
        case M_TYPE_UINT8:
        case M_TYPE_UINT32:
        case M_TYPE_INT32:
            if (M_INT32(val) > INT32_MAX) {
                jchild = NULL;
                break;
            }
            jchild = json_object_new_int(M_INT32(val));
            break;

        case M_TYPE_STRING:
            jchild = json_object_new_string(M_STR(val));
            break;
        case M_TYPE_ARRAY:
            jchild = json_object_new_array();
            chno_for_each(val, _chno_to_json_iter, (void*)jchild);
            break;
        default:
            jchild = NULL;
            break;
    } /* switch */

    if (jchild == NULL) {
        return -1;
    }

    if (key != NULL) {
        json_object_object_add(jparent, key, jchild);
    } else {
        json_object_array_add(jparent, jchild);
    }

    return 0;
} /* _chno_to_json_iter */

json_object *
chno_to_json(chno_t * m) {
    json_object * jobj;

    switch (chno_type(m)) {
        case M_TYPE_ARRAY:
            jobj = json_object_new_array();
            break;
        case M_TYPE_MAP:
            jobj = json_object_new_object();
            break;
        default:
            printf("mm2json must start with map or list\n");
            return NULL;
    }

    chno_for_each(m, _chno_to_json_iter, (void*)jobj);

    return jobj;
}

