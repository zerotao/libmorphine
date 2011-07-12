# - Try to find the lua and luajit libraries
# Once done this will define
#
# LUA_FOUND - System has lua
# LUA_INCLUDE_DIR - the lua include directory
# LUA_LIBRARIES 0 The libraries needed to link lua

# FIND_PATH(LIBEVENT_INCLUDE_DIR NAMES event.h)
FIND_PATH(LUA_INCLUDE_DIR NAMES lua.h PATH_SUFFIXES lua5.1)
FIND_LIBRARY(LUA_LIBRARIES NAMES lua5.1)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LUA DEFAULT_MSG LUA_LIBRARIES LUA_INCLUDE_DIR)

MARK_AS_ADVANCED(LUA_INCLUDE_DIR LUA_LIBRARIES)
