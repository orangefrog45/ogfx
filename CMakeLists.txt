cmake_minimum_required(VERSION 4.1)
project(ogfx-core)

set(CMAKE_CXX_STANDARD 23)

set(VK_BOOTSTRAP_VULKAN_HEADER_DIR $ENV{VULKAN_SDK}/Include)
add_subdirectory(extern/vk-bootstrap)

find_package(Vulkan REQUIRED)
add_subdirectory(extern/VulkanMemoryAllocator)
add_subdirectory(extern/volk)
add_subdirectory(extern/glfw)
add_subdirectory(extern/spirv-cross)

set(ogfx-core-includes
    ${CMAKE_CURRENT_LIST_DIR}/extern/vk-bootstrap/src
    ${CMAKE_CURRENT_LIST_DIR}/extern/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_LIST_DIR}/extern/glfw/include
    ${CMAKE_CURRENT_LIST_DIR}/extern/glm
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui
    ${CMAKE_CURRENT_LIST_DIR}/extern
    $ENV{VULKAN_SDK}/Include
)

set(ogfx-core-libs
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator
    volk
    glfw
    ${Vulkan_LIBRARIES}
    spirv-cross-cpp
)

add_subdirectory(core)
add_subdirectory(demo)

