#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <morphine.h>

#ifndef lua_open
# define lua_open luaL_newstate
#endif /* lua_open */

static void
register_functions(lua_State * L) {
    lua_register(L, "chno_from_lua", chno_from_lua_raw);
    lua_register(L, "chno_to_lua", chno_to_lua);
}

int
main(int argc, char ** argv) {
    lua_State * L;

    if (argc < 2) {
        fprintf(stderr, "Usage %s <lua script>\n", argv[0]);
        exit(1);
    }

    L = lua_open();

    luaL_openlibs(L);
    register_functions(L);
    (void)luaL_dofile(L, argv[1]);
    lua_close(L);

    return 0;
}

