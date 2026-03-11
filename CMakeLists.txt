cmake_minimum_required(VERSION 3.16...4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(junkcraft VERSION 1.0
    DESCRIPTION "Minecraft clone"
  LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)



add_subdirectory(src)
target_compile_options(junkcraft PRIVATE
    -Wall
    -Wextra
    -Wdouble-promotion
    -Wconversion
    -Wno-sign-conversion
    -Wno-unused-parameter
    -Wno-unused-function
    # -fsanitize=address,undefined
    -g3
)
# target_link_options(junkcraft PRIVATE
#     -fsanitize=address,undefined
# )
find_package(SDL2)
find_package(Freetype REQUIRED)
find_library(JUNK junk)
find_path(JUNK_INCLUDE "junk/vector.h")
target_include_directories(junkcraft PRIVATE include ${JUNK_INCLUDE} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(junkcraft PRIVATE SDL2 ${JUNK} m pthread freetype)
