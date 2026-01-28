cmake_minimum_required(VERSION 3.16...4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(junkcraft VERSION 1.0
    DESCRIPTION "Minecraft clone"
  LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)
message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_subdirectory(src)
target_include_directories(junkcraft PRIVATE include)
find_package(SDL2)
find_library(JUNK junk)
target_link_libraries(junkcraft PRIVATE SDL2 ${JUNK} m pthread)
