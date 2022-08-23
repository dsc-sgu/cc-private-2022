cmake_minimum_required(VERSION 3.0)
project (raygui)
add_library (raygui INTERFACE)
target_include_directories (raygui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

