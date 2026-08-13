#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <format>
#include <fstream>
#include <sstream>
#include <stacktrace>
#include <optional>
#include <chrono>

#include <lml/core.h>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

#include "ogfx/util/Logger.h"
#include "ogfx/util/VkUtil.h"
