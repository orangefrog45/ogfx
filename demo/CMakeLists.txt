cmake_minimum_required(VERSION 4.1)

project(ogfx-demo)

add_executable(ogfx-demo
    src/main.cpp
)

target_link_libraries(ogfx-demo PUBLIC ogfx-core)

target_compile_definitions(ogfx-demo PUBLIC DEMO_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/shaders")