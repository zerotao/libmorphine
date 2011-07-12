cmake_minimum_required(VERSION 2.8)
project(morphine)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include(BaseConfig)

message("BUILD TYPE: ${CMAKE_BUILD_TYPE}")
message("Std CFLAGS: ${CMAKE_C_FLAGS}")
message("Debug FLAGS: ${CMAKE_C_FLAGS_DEBUG}")
message("Release FLAGS: ${CMAKE_C_FLAGS_RELEASE}")
message("MinSizeRel FLAGS: ${CMAKE_C_FLAGS_MINSIZEREL}")
message("RelWithDebInfo FLAGS: ${CMAKE_C_FLAGS_RELWITHDEBINFO}")

find_package(LibZt)
find_package(LibEvent)
find_package(LibJSONC)
find_package(Lua)
find_package(LuaLib)
find_package(LibZ)

set (MORPHINE_SOURCES
	morphine.c
  morph/json.c
  morph/lua.c
  morph/stdout.c
)

set (MORPHINE_HEADERS
	morphine.h
	include/base.h
	include/morph.h
	include/morph/json.h
	include/morph/lua.h
	include/morph/stdout.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${LIBZT_INCLUDE_DIR}
	${LIBEVENT_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
  ${LUALIB_INCLUDE_DIR}
	${LIBJSONC_INCLUDE_DIR}
  ${LIBZ_INCLUDE_DIR}
)

set(MORPHINE_EXTERNAL_LIBS
	${LIBZT_LIBRARIES}
	${LIBEVENT_LIBRARY}
	${LUA_LIBRARIES}
  ${LIALUB_LIBRARIES}
	${LIBJSONC_LIBRARIES}
  ${LIBZ_LIBRARIES}
)

add_library(libmorphine STATIC ${MORPHINE_SOURCES})
set_target_properties(libmorphine PROPERTIES OUTPUT_NAME "morphine")
target_link_libraries(libmorphine ${MORPHINE_EXTERNAL_LIBS})


add_executable(maps ${CMAKE_CURRENT_SOURCE_DIR}/examples/maps.c)

target_link_libraries(maps libmorphine)
