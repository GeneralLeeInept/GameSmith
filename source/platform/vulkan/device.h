#pragma once

#include "gsvulkan.h"

namespace GameSmith
{
namespace Vulkan
{

void ChoosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice, uint32_t& graphicsQueueIndex);
VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueIndex);

}
} // namespace GameSmith