# - Try to find the Libzt config processing library
# Once done this will define
#
# LIBZT_FOUND - System has Libzt
# LIBZT_INCLUDE_DIR - the Libzt include directory
# LIBZT_LIBRARIES 0 The libraries needed to use Libzt
# LIBZT_UNIT_LIBRARIES 0 The libraries needed to use Libzt UNIT
# LIBZT_GC_LIBRARIES 0 The libraries needed to use Libzt GC

FIND_PATH(LIBZT_INCLUDE_DIR NAMES zt.h PATH_SUFFIXES libzt)
FIND_LIBRARY(LIBZT_LIBRARIES NAMES zt)
FIND_LIBRARY(LIBZT_UNIT_LIBRARIES NAMES zt_unit)
FIND_LIBRARY(LIBZT_GC_LIBRARIES NAMES zt_gc)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libzt DEFAULT_MSG LIBZT_LIBRARIES
    LIBZT_INCLUDE_DIR)


find_package(Threads)
include(CheckCSourceCompiles)

set(LIBZT_LIBRARIES ${LIBZT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

MARK_AS_ADVANCED(LIBZT_INCLUDE_DIR LIBZT_LIBRARIES LIBZT_UNIT_LIBRARIES)
