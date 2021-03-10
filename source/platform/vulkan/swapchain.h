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

bool CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, Swapchain& swapchain);
void DestroySwapchain(VkDevice device, Swapchain& swapchain);

} // namespace Vulkan
} // namespace GameSmith
