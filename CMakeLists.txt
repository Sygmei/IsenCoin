cmake_minimum_required(VERSION 3.6)

project(isencoin)

add_subdirectory(extlibs/ed25519)
add_subdirectory(extlibs/base58)
add_subdirectory(extlibs/msgpack11)
add_subdirectory(extlibs/tacopie)
add_subdirectory(extlibs/fmt)
add_subdirectory(extlibs/spdlog)
add_subdirectory(extlibs/vili)
add_subdirectory(extlibs/imgui)

find_package(Threads REQUIRED)
find_package(SFML 2 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB ISENCOIN_HEADERS include/*.hpp)
file(GLOB ISENCOIN_SOURCES src/*.cpp)

add_executable(isencoin ${ISENCOIN_HEADERS} ${ISENCOIN_SOURCES})

target_link_libraries(isencoin ed25519)
target_link_libraries(isencoin base58)
target_link_libraries(isencoin msgpack11)
target_link_libraries(isencoin tacopie)
target_link_libraries(isencoin fmt)
target_link_libraries(isencoin spdlog)
target_link_libraries(isencoin vili)
target_link_libraries(isencoin imgui)
target_link_libraries(isencoin Threads::Threads)
if (SFML_FOUND)
  target_link_libraries(isencoin sfml-window sfml-graphics sfml-system)
  target_link_libraries(isencoin ${OPENGL_LIBRARIES})
endif()

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