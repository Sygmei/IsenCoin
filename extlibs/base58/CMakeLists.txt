cmake_minimum_required(VERSION 3.6)

project(base58)

file(GLOB B58_HEADERS include/base58/*.hpp)
file(GLOB B58_SOURCES src/*.cpp)

add_library(base58 ${B58_HEADERS} ${B58_SOURCES})

target_include_directories(base58
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/base58
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)