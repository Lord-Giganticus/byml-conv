cmake_minimum_required(VERSION 3.10)
project(byml_conv CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_TESTING OFF)

add_executable(byml_conv byml_handler.hpp sarc_handler.hpp main.cpp)
if (MSVC)
    target_compile_options(byml_conv PRIVATE /Zc:__cplusplus)
endif()

add_subdirectory(oead)

find_package(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(byml_conv PUBLIC oead Boost::program_options)