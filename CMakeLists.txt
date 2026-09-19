cmake_minimum_required(VERSION 3.16...4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(junkcraft VERSION 1.0
    DESCRIPTION "Minecraft clone"
  LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)


option(BUILD_STATIC "Compile statically" ON)

add_subdirectory(src)
target_compile_options(junkcraft PRIVATE
    -Wall
    -Wextra
    -Wno-double-promotion
    -Wno-conversion
    -Wno-sign-conversion
    -Wno-unused-parameter
    -Wno-unused-function
    # -fsanitize=address,undefined
    -g3
)
# target_link_options(junkcraft PRIVATE
#     -fsanitize=address,undefined
# )
if(BUILD_STATIC)
find_package(PkgConfig)
find_package(Freetype REQUIRED)
find_path(JUNK_INCLUDE "junk/vector.h")
target_link_libraries(junkcraft PRIVATE m libjunk.a libpthread.a)
target_link_libraries(junkcraft PRIVATE libSDL2.a)
target_link_libraries(junkcraft PRIVATE libz.a)
target_link_libraries(junkcraft PRIVATE libfreetype.a)
target_include_directories(junkcraft PRIVATE include ${ZLIB_INCLUDE_DIR} ${JUNK_INCLUDE} ${FREETYPE_INCLUDE_DIRS})
else()
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_library(JUNK junk REQUIRED)
find_path(JUNK_INCLUDE "junk/vector.h")
target_link_libraries(junkcraft PRIVATE SDL2 ${JUNK} m pthread freetype)
target_include_directories(junkcraft PRIVATE include ${JUNK_INCLUDE} ${FREETYPE_INCLUDE_DIRS})
endif()

target_compile_options(junkcraft-server PRIVATE
    -Wall
    -Wextra
    -Wno-double-promotion
    -Wno-conversion
    -Wno-sign-conversion
    -Wno-unused-parameter
    -Wno-unused-function
    # -fsanitize=address,undefined
    -g3
)
# target_link_options(junkcraft-server PRIVATE
#     -fsanitize=address,undefined
# )
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_library(JUNK junk REQUIRED)
find_path(JUNK_INCLUDE "junk/vector.h")
target_include_directories(junkcraft-server PRIVATE include ${JUNK_INCLUDE} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(junkcraft-server PRIVATE SDL2 ${JUNK} m pthread freetype)
