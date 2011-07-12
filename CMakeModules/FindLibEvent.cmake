# - Try to find the LibEvent config processing library
# Once done this will define
#
# LIBEVENT_FOUND - System has LibEvent
# LIBEVENT_INCLUDE_DIR - the LibEvent include directory
# LIBEVENT_LIBRARIES 0 The libraries needed to use LibEvent

FIND_PATH(LIBEVENT_INCLUDE_DIR NAMES event.h)
FIND_LIBRARY(LIBEVENT_LIBRARY NAMES event)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibEvent DEFAULT_MSG LIBEVENT_LIBRARY LIBEVENT_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBEVENT_INCLUDE_DIR LIBEVENT_LIBRARY)
