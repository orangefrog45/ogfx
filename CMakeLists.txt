cmake_minimum_required(VERSION 4.1)
project(ogfx-core)

set(CMAKE_CXX_STANDARD 23)

set(VK_BOOTSTRAP_VULKAN_HEADER_DIR $ENV{VULKAN_SDK}/Include)
add_subdirectory(extern/vk-bootstrap)
add_subdirectory(extern/otl)

find_package(Vulkan REQUIRED)
add_subdirectory(extern/VulkanMemoryAllocator)
add_subdirectory(extern/volk)
add_subdirectory(extern/glfw)
add_subdirectory(extern/spirv-cross)

set(ogfx-core-includes
        ${CMAKE_CURRENT_LIST_DIR}/extern/vk-bootstrap/src
        ${CMAKE_CURRENT_LIST_DIR}/extern/VulkanMemoryAllocator/include
        ${CMAKE_CURRENT_LIST_DIR}/extern/glfw/include
        ${CMAKE_CURRENT_LIST_DIR}/extern/lml/include
        ${CMAKE_CURRENT_LIST_DIR}/extern/imgui
        ${CMAKE_CURRENT_LIST_DIR}/extern
        ${CMAKE_CURRENT_LIST_DIR}/extern/otl/include
        ${CMAKE_CURRENT_LIST_DIR}/../../extern/entt/src
        ${CMAKE_CURRENT_LIST_DIR}/core/include
        $ENV{VULKAN_SDK}/Include
        CACHE STRING "" FORCE
)

set(ogfx-core-libs
        vk-bootstrap::vk-bootstrap
        VulkanMemoryAllocator
        volk
        glfw
        otl
        ${Vulkan_LIBRARIES}
        spirv-cross-cpp
        CACHE STRING "" FORCE
)

set(ogfx-core-compile-options
        $<$<CXX_COMPILER_ID:MSVC>:
        /Zc:preprocessor
        /WX
        /W4
        >
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Werror
        -Wall
        -Wextra
        -Wno-documentation-deprecated-sync
        -Wno-c++98-compat-pedantic
        -Wno-c++98-compat
        -Wno-gnu-anonymous-struct
        -Wno-documentation
        -Wno-nested-anon-types
        -Wno-switch-default
        -Wno-old-style-cast
        -Wno-unsafe-buffer-usage
        -Wno-c++20-compat
        -Wno-missing-designated-field-initializers
        -Wno-missing-field-initializers
        -Wno-exit-time-destructors
        -Wno-sign-conversion
        >
)

add_subdirectory(core)
add_subdirectory(demo)

