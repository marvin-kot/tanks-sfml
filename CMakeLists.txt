cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER "C:/Program Files/w64devkit/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files/w64devkit/bin/g++.exe")

project(pure_cpp)

include_directories("D:/projects/gamedev/sfml/include")

link_directories("D:/projects/gamedev/sfml/lib")

link_libraries(sfml-graphics sfml-window sfml-system)

set(CMAKE_BUILD_TYPE Debug)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)


add_executable(game WIN32 main.cpp GameObject.cpp SpriteRenderer.cpp Controller.cpp)

target_link_libraries(game PRIVATE nlohmann_json::nlohmann_json)



