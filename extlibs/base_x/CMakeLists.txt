cmake_minimum_required(VERSION 3.6)

project(basex)

file(GLOB BASEX_HEADERS include/base_x/*.hh)

add_library(basex INTERFACE)

target_include_directories(basex
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)