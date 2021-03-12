#pragma once

#include "gsvulkan.h"

namespace gs
{

namespace vk
{

struct Swapchain
{
    VkSwapchainKHR swapchain;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
};

bool createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, Swapchain& swapchain);
void destroySwapchain(VkDevice device, Swapchain& swapchain);

} // namespace Vulkan
} // namespace GameSmith
