#pragma once

#include "gamesmith/core/config.h"

#ifdef GS_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#ifndef GS_RELEASE
#define GS_VULKAN_VALIDATION 1
#else
#define GS_VULKAN_VALIDATION 0
#endif

#define VK_CHECK_RESULT(call)            \
    do                                   \
    {                                    \
        VkResult result = (call);        \
        GS_ASSERT(result == VK_SUCCESS); \
    } while (0)
