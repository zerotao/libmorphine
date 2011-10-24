FIND_PATH(LUALIB_INCLUDE_DIR NAMES lualib.h PATH_SUFFIXES lua-5.1 lua-5.1 "")
FIND_LIBRARY(LUALIB_LIBRARIES NAMES lualib-5.1 lualib5.1 lualib)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LUALIB DEFAULT_MSG LUALIB_LIBRARIES LUALIB_INCLUDE_DIR)

MARK_AS_ADVANCED(LUALIB_INCLUDE_DIR LUALIB_LIBRARIES)
