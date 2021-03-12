#pragma once

#include "gsvulkan.h"

namespace gs
{
namespace vk
{

void choosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice, uint32_t& graphicsQueueIndex);
VkDevice createDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueIndex);

}
} // namespace GameSmith