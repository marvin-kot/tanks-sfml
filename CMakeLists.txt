cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_COMPILER "C:/Program Files/w64devkit/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files/w64devkit/bin/g++.exe")

project(tankcpp)
set(CMAKE_CXX_STANDARD 20)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(lib/json)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/lib/sfml/include)
set(SFML_LIB_DIR ${CMAKE_SOURCE_DIR}/lib/sfml/bin)
link_directories(${SFML_LIB_DIR})
link_libraries(sfml-graphics sfml-window sfml-system sfml-audio)

file(GLOB ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

# sftools
include_directories(${CMAKE_SOURCE_DIR}/lib/sftools/include)

add_executable(game WIN32 main.cpp ${ALL_SOURCES})
#add_executable(visualizer WIN32 visualizer.cpp ${ALL_SOURCES})

target_link_libraries(game PRIVATE nlohmann_json::nlohmann_json)
#target_link_libraries(visualizer PRIVATE nlohmann_json::nlohmann_json)

add_custom_target(copy-assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    DEPENDS game #visualizer
)

add_custom_target(copy-libs ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-graphics-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-window-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-system-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-audio-2.dll ${CMAKE_BINARY_DIR}
    DEPENDS game #visualizer
)

set(CMAKE_BUILD_TYPE Debug)
