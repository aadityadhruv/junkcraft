cmake_minimum_required(VERSION 3.16...4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(junkcraft VERSION 1.0
    DESCRIPTION "Minecraft clone"
  LANGUAGES C)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_subdirectory(src)
target_include_directories(junkcraft PRIVATE include)
find_package(SDL2)
target_link_libraries(junkcraft PRIVATE SDL2)
