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
find_package(LibJSONC)
find_package(Lua)
#find_package(LuaLib)
find_package(LibZ)

include(CheckDefIncludeFiles)

set (MORPHINE_SOURCES
	morphine.c
    util/buf.c
    morph/json.c
    morph/lua.c
    morph/stdout.c
    )

set (MORPHINE_HEADERS
	morphine.h
	include/base.h
    include/util/buf.h
	include/morph.h
	include/morph/json.h
	include/morph/lua.h
	include/morph/stdout.h
    )

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${LIBZT_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
    ${LUALIB_INCLUDE_DIR}
	${LIBJSONC_INCLUDE_DIR}
    ${LIBZ_INCLUDE_DIR}
    )

set(MORPHINE_EXTERNAL_LIBS
	${LIBZT_LIBRARIES}
	${LIBJSONC_LIBRARIES}
    ${LIBZ_LIBRARIES}
    )

add_library(libmorphine STATIC ${MORPHINE_SOURCES})
set_target_properties(libmorphine PROPERTIES OUTPUT_NAME "morphine")
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SYS_EXTRA_LIBS Ws2_32)
endif()

target_link_libraries(libmorphine ${MORPHINE_EXTERNAL_LIBS} ${SYS_EXTRA_LIBS})

install (TARGETS libmorphine DESTINATION lib)
install (FILES morphine.h DESTINATION include)
install (FILES include/morphine/base.h DESTINATION include/morphine)
install (FILES include/morphine/morph.h DESTINATION include/morphine)
install (FILES include/morphine/morph/json.h DESTINATION include/morphine/morph)
install (FILES include/morphine/morph/lua.h DESTINATION include/morphine/morph)
install (FILES include/morphine/morph/stdout.h DESTINATION include/morphine/morph)
install (FILES include/morphine/util.h DESTINATION include/morphine)
install (FILES include/morphine/util/buf.h DESTINATION include/morphine/util)


add_executable(chno_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/chno_example.c)
target_link_libraries(chno_example libmorphine)
add_executable(chno_lua_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/chno_lua_example.c)
target_link_libraries(chno_lua_example libmorphine ${LUA_LIBRARIES} ${LUALIB_LIBRARIES})
add_executable(chno_buffer_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/chno_buffer_example.c)
target_link_libraries(chno_buffer_example libmorphine)
