cmake_minimum_required(VERSION 4.1)

project(ogfx)

add_library(ogfx-core STATIC
    src/VkContext.cpp
    src/resources/Image.cpp
    src/VkCommon.cpp
    src/Window.cpp
    src/Gui.cpp
    src/shaders/Shader.cpp
)

add_library(imgui STATIC
    ../extern/imgui/imgui.cpp
    ../extern/imgui/imgui_draw.cpp
    ../extern/imgui/imgui_demo.cpp
    ../extern/imgui/imgui_tables.cpp
    ../extern/imgui/imgui_widgets.cpp
    ../extern/imgui/backends/imgui_impl_vulkan.cpp
    ../extern/imgui/backends/imgui_impl_glfw.cpp
    ../extern/imgui/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui SYSTEM PUBLIC
    include
    ${ogfx-core-includes}
    ../extern/volk
)

target_compile_options(ogfx-core PRIVATE ${ogfx-core-compile-options})

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)

target_include_directories(ogfx-core PUBLIC
    include
)

target_include_directories(ogfx-core SYSTEM PUBLIC
    ${ogfx-core-includes}
)

target_link_libraries(ogfx-core PUBLIC
    ${ogfx-core-libs}
    imgui
)

target_precompile_headers(ogfx-core PRIVATE include/ogfx/pch.h)
