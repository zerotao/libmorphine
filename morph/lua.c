#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <zt.h>

#include "morphine/util.h"
#include "morphine/base.h"
#include "morphine/morph/lua.h"

static int chno_lua_iter_map(const char * key, chno_t * val, void * arg);
static int chno_lua_push_node(lua_State * L, chno_t * val);

static chno_t *
chno_from_lua_s(lua_State * L) {
    chno_t * m = NULL;

    lua_pushnil(L);

    while (lua_next(L, -2)) {
        chno_t * mbr;

        if (m == NULL) {
            if (lua_type(L, -2) == LUA_TNUMBER) {
                m = chno_array_new();
            } else {
                m = chno_map_new();
            }
        }

        switch (lua_type(L, -1)) {
            case LUA_TNUMBER:
                if (lua_tointeger(L, -1) > INT32_MAX) {
                    return NULL;
                }
                mbr = chno_int32_new((int32_t)lua_tointeger(L, -1));
                break;
            case LUA_TSTRING:
                mbr = chno_string_new(lua_tostring(L, -1));
                break;
            case LUA_TTABLE:
                mbr = chno_from_lua_s(L);
                break;
        }

        lua_pop(L, 1);

        if (lua_type(L, -1) == LUA_TSTRING) {
            if (chno_valid_type(m, M_TYPE_ARRAY) == true) {
                chno_t * mmap = chno_map_new();

                chno_add(mmap, mbr, lua_tostring(L, -1));
                chno_add(m, mmap, NULL);
            } else {
                chno_add(m, mbr, lua_tostring(L, -1));
            }
        } else {
            chno_add(m, mbr, NULL);
        }
    }

    return m;
} /* chno_from_lua */

chno_t *
chno_from_lua(lua_State * L) {
    bool     encap = false;
    chno_t * ret   = NULL;

    if (lua_type(L, -1) != LUA_TTABLE) {
        /*
         * the parent type is not a table, create a encapsulating
         * array, which should be removed before returning
         */
        encap = true;

        lua_newtable(L);
        lua_pushinteger(L, 1);
        lua_pushvalue(L, -3);
        lua_remove(L, -4);
        lua_settable(L, -3);
    }

    if (!(ret = chno_from_lua_s(L))) {
        return NULL;
    }

    if (encap == true) {
        /*
         * this data was wrapped in an array, so remove it
         * and return the real value
         */
        chno_t * v;

        if (!(v = chno_copy(chno_array_get(ret, 0)))) {
            chno_free(ret);
            return NULL;
        }

        chno_free(ret);
        ret = v;
    }

    return ret;
}

int
chno_from_lua_raw(lua_State * L) {
    chno_t * m = chno_from_lua(L);

    lua_pushlightuserdata(L, m);
    return 1;
}

struct li {
    int         index;
    lua_State * L;
};

static int
chno_lua_push_node(lua_State * L, chno_t * val) {
    struct li ll;
    int       err = 0;

    switch (chno_type(val)) {
	case M_TYPE_INT64:
	case M_TYPE_UINT64:
        case M_TYPE_INT32:
        case M_TYPE_UINT32:
        case M_TYPE_UINT16:
        case M_TYPE_INT16:
        case M_TYPE_UINT8:
        case M_TYPE_INT8:
            lua_pushnumber(L, chno_int32(val, &err, NULL));
            return 0;
        case M_TYPE_STRING:
            lua_pushstring(L, chno_string(val, &err, NULL));
            return 0;
        case M_TYPE_RAW:
            lua_pushlstring(L, M_RAW_DATA(val), M_RAW_LEN(val));
            return 0;
        case M_TYPE_MAP:
        case M_TYPE_ARRAY:
            ll.L     = L;
            ll.index = 1;
            lua_newtable(L);
            chno_for_each(val, chno_lua_iter_map, (void *)&ll);
            return 0;
        default:
            return -1;
    } /* switch */

    return 0;
}

static int
chno_lua_iter_map(const char * key, chno_t * val, void * arg) {
    struct li * ll = (struct li*)arg;
    lua_State * L  = ll->L;

    if (key == NULL) {
        lua_pushnumber(L, ll->index++);
    } else {
        lua_pushstring(L, key);
    }

    chno_lua_push_node(L, val);
    lua_settable(L, -3);

    return 0;
}

int
chno_to_lua_push(lua_State * L, chno_t * m) {
    chno_lua_push_node(L, m);
    return 1;
}

int
chno_to_lua(lua_State * L) {
    chno_t * m;

    m = (chno_t *)lua_topointer(L, -1);

    lua_pop(L, 1);

    chno_lua_push_node(L, m);
    return 1;
}

