#include "gspch.h"

#include "swapchain.h"

#include "gamesmith/core/core.h"
#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"

namespace GameSmith
{
namespace Vulkan
{
VkSwapchainKHR CreateSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, VkSurfaceFormatKHR surfaceFormat)
{
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
    GS_ASSERT(surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.surface = surface;
    createInfo.minImageCount = std::max(2u, surfaceCaps.minImageCount);
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = surfaceCaps.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: support other composite alpha
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_FALSE; // TODO: mindfulness

    VkSwapchainKHR swapchain{};
    VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));
    return swapchain;
}

bool CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, Swapchain& swapchain)
{
    swapchain.swapchain = CreateSwapchain(physicalDevice, surface, device, surfaceFormat);

    if (!swapchain.swapchain)
    {
        return false;
    }

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
    swapchain.extent = surfaceCaps.currentExtent;

    uint32_t swapchainImageCount{};
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchainImageCount, nullptr));
    swapchain.images.resize(swapchainImageCount);
    swapchain.imageViews.resize(swapchainImageCount);

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchainImageCount, swapchain.images.data()));

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.image = swapchain.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        createInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &swapchain.imageViews[i]));
    }

    return true;
}

void DestroySwapchain(VkDevice device, Swapchain& swapchain)
{
    for (VkImageView& imageView : swapchain.imageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    if (swapchain.swapchain)
    {
        vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
    }

    swapchain = Swapchain{};
}
} // namespace Vulkan
} // namespace GameSmith
