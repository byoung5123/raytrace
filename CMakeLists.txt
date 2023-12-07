cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
   set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()
# our project name
project(SDLPutPixelExample)
# Now find the fmt library
find_package(SDL2 CONFIG REQUIRED)
# Set to C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(raytrace)
target_sources(raytrace PRIVATE main.cpp )
target_compile_options(raytrace PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(raytrace PRIVATE  SDL2::SDL2 SDL2::SDL2main )

