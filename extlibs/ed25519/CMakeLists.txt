cmake_minimum_required(VERSION 3.6)

project(ed25519)

file(GLOB ED25519_HEADERS include/ed25519/*.h)
file(GLOB ED25519_SOURCES src/*.c)

add_library(ed25519 ${ED25519_HEADERS} ${ED25519_SOURCES})

if (MSVC)
	target_compile_definitions(ed25519 PUBLIC _STDINT_H_ _STDINT_H_INCLUDED __WATCOMC__)
endif()

target_include_directories(ed25519
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ed25519
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)