# - Try to find the Libjsonc config processing library
# Once done this will define
#
# LIBJSONC_FOUND - System has Libjsonc
# LIBJSONC_INCLUDE_DIR - the Libjsonc include directory
# LIBJSONC_LIBRARIES 0 The libraries needed to use Libjsonc

FIND_PATH(LIBJSONC_INCLUDE_DIR NAMES json.h PATH_SUFFIXES json)
FIND_LIBRARY(LIBJSONC_LIBRARIES NAMES json)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibJSONC DEFAULT_MSG LIBJSONC_LIBRARIES
    LIBJSONC_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBJSONC_INCLUDE_DIR LIBJSONC_LIBRARIES)
