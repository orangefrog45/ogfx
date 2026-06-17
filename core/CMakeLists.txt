cmake_minimum_required(VERSION 4.1)

project(ogfx)

add_library(ogfx-core STATIC
    src/util/Logger.cpp
    src/VkContext.cpp
    src/resources/Image.cpp
    src/VkCommon.cpp
    src/util/Util.cpp
    src/Window.cpp
    src/Gui.cpp
    src/shaders/Shader.cpp

    ../extern/imgui/imgui.cpp
    ../extern/imgui/imgui_draw.cpp
    ../extern/imgui/imgui_demo.cpp
    ../extern/imgui/imgui_tables.cpp
    ../extern/imgui/imgui_widgets.cpp
    ../extern/imgui/backends/imgui_impl_vulkan.cpp
    ../extern/imgui/backends/imgui_impl_glfw.cpp
)


target_include_directories(ogfx-core PUBLIC
    include
    ${ogfx-core-includes}
)

target_link_libraries(ogfx-core PUBLIC
    ${ogfx-core-libs}
)

target_precompile_headers(ogfx-core PRIVATE include/ogfx/pch.h)
