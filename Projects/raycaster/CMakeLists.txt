cmake_minimum_required(VERSION 3.0)

get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} C CXX)

set (CMAKE_CXX_STANDARD 17)
add_executable (${PROJECT_NAME} main.cpp)
target_link_libraries (${PROJECT_NAME} LINK_PRIVATE raylib-ext)
target_link_libraries (${PROJECT_NAME} LINK_PRIVATE glad)
target_link_libraries (${PROJECT_NAME} LINK_PRIVATE glfw)

