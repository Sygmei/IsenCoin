cmake_minimum_required(VERSION 3.6)

project(cryptopp)

file(GLOB CRYPTOPP_HEADERS include/*.h)
file(GLOB CRYPTOPP_SOURCES src/*.cpp)

add_library(cryptopp ${CRYPTOPP_HEADERS} ${CRYPTOPP_SOURCES})

target_include_directories(cryptopp
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/cryptopp
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif (MSVC)