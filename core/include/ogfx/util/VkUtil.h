#pragma once

#include "ogfx/util/Logger.h"
#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>
#include <stacktrace>

#define OGFX_VK_CHECK(x) \
    do { \
        auto res = static_cast<vk::Result>(x); \
        if (res != vk::Result::eSuccess) { \
            OGFX_CORE_ERROR("Vulkan error at {}:{}: {} - {}", __FILE__, __LINE__, #x, string_VkResult(static_cast<VkResult>(res))); \
            OGFX_BREAKPOINT; \
        } \
    } while (false)

