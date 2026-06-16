cmake_minimum_required(VERSION 4.1)

project(ogfx-demo)

add_executable(ogfx-demo
    src/main.cpp
)

target_link_libraries(ogfx-demo PUBLIC ogfx-core)