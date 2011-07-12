#ifndef __CHNO_MORPH_LUA_H__
#define __CHNO_MORPH_LUA_H__
/* includes */
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <zt.h>
#include <morphine.h>

BEGIN_C_DECLS

/* module declarations */
chno_t * chno_from_lua(lua_State * L);
int      chno_from_lua_raw(lua_State * L);
int      chno_to_lua(lua_State * L);
int      chno_to_lua_push(lua_State * L, chno_t * m);

END_C_DECLS

#endif

