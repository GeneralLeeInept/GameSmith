#include "gspch.h"

#include "device.h"

#include "gamesmith/core/core.h"
#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "renderer_vk.h"

namespace GameSmith
{
namespace Vulkan
{

void ChoosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice, uint32_t& graphicsQueueIndex)
{
    physicalDevice = VK_NULL_HANDLE;

    uint32_t physicalDeviceCount{};
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    std::vector<VkPhysicalDevice> suitableDevices;
    suitableDevices.reserve(physicalDeviceCount);

    std::vector<uint32_t> graphicsQueueIndices;
    graphicsQueueIndices.reserve(physicalDeviceCount);

    for (VkPhysicalDevice& device : physicalDevices)
    {
        uint32_t queueFamilyPropertiesCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertiesCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertiesCount, queueFamilyProperties.data());

        // Need graphics queue with presentation support
        std::optional<uint32_t> graphicsQueueIndex{};

        for (uint32_t q = 0; q < queueFamilyPropertiesCount; ++q)
        {
            if (!graphicsQueueIndex && (queueFamilyProperties[q].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                VkBool32 supported{};
                VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(device, q, surface, &supported));

                if (supported)
                {
                    graphicsQueueIndex = q;
                }
            }
        }

        if (graphicsQueueIndex)
        {
            suitableDevices.push_back(device);
            graphicsQueueIndices.push_back(graphicsQueueIndex.value());
        }
    }

    if (suitableDevices.size() > 0)
    {
        for (size_t i = 0; i < suitableDevices.size(); ++i)
        {
            VkPhysicalDevice& device = suitableDevices[i];
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(device, &props);

            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                physicalDevice = device;
                graphicsQueueIndex = graphicsQueueIndices[i];
                GS_INFO("Choosing discrete GPU: %s, graphics/presentation queue index: %u", props.deviceName, graphicsQueueIndex);
                break;
            }
        }

        if (!physicalDevice)
        {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(suitableDevices[0], &props);
            physicalDevice = suitableDevices[0];
            graphicsQueueIndex = graphicsQueueIndices[0];
            GS_INFO("Choosing fallback device: %s, graphics/presentation queue index: %u", props.deviceName, graphicsQueueIndex);
        }
    }
    else
    {
        GS_INFO("No suitable Vulkan device found.");
    }
}

VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueIndex)
{
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    static const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDeviceFeatures2 enabledFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilLayoutsFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES };
    enabledFeatures.pNext = &separateDepthStencilLayoutsFeatures;
    separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts = VK_TRUE;

    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext = &enabledFeatures;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledExtensionCount = GS_ARRAY_COUNT(extensions);

    VkDevice device{};
    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
    return device;
}

}
}