cmake_minimum_required(VERSION 3.6)

project(isencoin)

add_subdirectory(extlibs/ed25519)
add_subdirectory(extlibs/base_x)
add_subdirectory(extlibs/msgpack11)
add_subdirectory(extlibs/tacopie)

find_package(Threads REQUIRED)

file(GLOB ISENCOIN_HEADERS include/*.hpp)
file(GLOB ISENCOIN_SOURCES src/*.cpp)

add_executable(isencoin ${ISENCOIN_HEADERS} ${ISENCOIN_SOURCES})

target_link_libraries(isencoin ed25519)
target_link_libraries(isencoin basex)
target_link_libraries(isencoin msgpack11)
target_link_libraries(isencoin tacopie)
target_link_libraries(isencoin Threads::Threads)

target_include_directories(isencoin
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif (MSVC)