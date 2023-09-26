cmake_minimum_required(VERSION 3.27)

project(tankcpp VERSION 0.3.2)

set(CMAKE_CXX_STANDARD 20)

find_package(python)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(lib/json)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src)

# sfml
include_directories(${CMAKE_SOURCE_DIR}/lib/sfml/include)
set(SFML_LIB_DIR ${CMAKE_SOURCE_DIR}/lib/sfml/bin)
link_directories(${SFML_LIB_DIR})
link_libraries(sfml-graphics sfml-window sfml-system sfml-audio sfml-network)

file(GLOB ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

# sftools
include_directories(${CMAKE_SOURCE_DIR}/lib/sftools/include)


add_executable(game WIN32 main.cpp ${ALL_SOURCES})
#add_executable(visualizer WIN32 visualizer.cpp ${ALL_SOURCES})
#add_executable(server WIN32 main_server.cpp ${ALL_SOURCES})
#add_executable(client WIN32 main_client.cpp ${ALL_SOURCES})
#target_link_libraries(game PRIVATE )

target_compile_definitions(game PUBLIC SINGLE_APP)
#target_compile_definitions(server PUBLIC GAME_SERVER)
#target_compile_definitions(client PUBLIC GAME_CLIENT)

target_link_libraries(game PRIVATE
        nlohmann_json::nlohmann_json)
#target_link_libraries(server PRIVATE
#        nlohmann_json::nlohmann_json)
#target_link_libraries(client PRIVATE
#        nlohmann_json::nlohmann_json)

#target_link_libraries(visualizer PRIVATE nlohmann_json::nlohmann_json)

add_custom_target(run-scripts ALL
    COMMAND python ${CMAKE_SOURCE_DIR}/scripts/enumeratespritesheet.py
)

add_custom_target(copy-assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    DEPENDS game #server #client #visualizer
)

add_custom_target(copy-libs ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-graphics-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-window-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-system-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-audio-2.dll ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SFML_LIB_DIR}/sfml-network-2.dll ${CMAKE_BINARY_DIR}
    DEPENDS game #server #client #visualizer
)

set(CMAKE_BUILD_TYPE Debug)
