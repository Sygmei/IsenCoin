cmake_minimum_required(VERSION 3.6)

project(tacopie)

file(GLOB_RECURSE TACOPIE_HEADERS include/tacopie/*.hpp)
file(GLOB_RECURSE TACOPIE_SOURCES src/*.cpp)

add_library(tacopie ${TACOPIE_HEADERS} ${TACOPIE_SOURCES})

target_compile_definitions(tacopie PUBLIC __TACOPIE_LOGGING_ENABLED)
target_include_directories(tacopie
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tacopie
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)