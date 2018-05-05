cmake_minimum_required(VERSION 3.6)

project(msgpack11)

file(GLOB MSGPACK11_HEADERS include/msgpack11/*.hpp)
file(GLOB MSGPACK11_SOURCES src/*.cpp)

add_library(msgpack11 ${MSGPACK11_HEADERS} ${MSGPACK11_SOURCES})

target_include_directories(msgpack11
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/msgpack11
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)