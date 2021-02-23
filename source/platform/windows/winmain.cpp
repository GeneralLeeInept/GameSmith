#include "gspch.h"

#include "gamesmith/core/core.h"
#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "gamesmith/core/window.h"

#ifdef GS_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(call)            \
    do                                   \
    {                                    \
        VkResult result = (call);        \
        GS_ASSERT(result == VK_SUCCESS); \
    } while (0)

struct ComHelper
{
    ComHelper() { CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); }
    ~ComHelper() { CoUninitialize(); }
};

VkInstance CreateInstance()
{
    VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "GameSmith Application";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName = "GameSmith";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &applicationInfo;

#ifndef GS_RELEASE
    static const char* enabledLayers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = GS_ARRAY_COUNT(enabledLayers);
    createInfo.ppEnabledLayerNames = enabledLayers;
#endif

    static const char* enabledExtensions[] = { VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef GS_PLATFORM_WINDOWS
                                               VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
    };

    createInfo.enabledExtensionCount = GS_ARRAY_COUNT(enabledExtensions);
    createInfo.ppEnabledExtensionNames = enabledExtensions;

    VkInstance instance{};
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    return instance;
}

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

    VkPhysicalDeviceFeatures enabledFeatures{};

    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &enabledFeatures;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledExtensionCount = GS_ARRAY_COUNT(extensions);

    VkDevice device{};
    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
    return device;
}

VkSurfaceKHR CreateSurface(VkInstance instance, HINSTANCE hInstance, HWND hWnd)
{
    VkWin32SurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    createInfo.hinstance = hInstance;
    createInfo.hwnd = hWnd;

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
    return surface;
}

VkSurfaceFormatKHR ChooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t surfaceFormatCount{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()));
    VkSurfaceFormatKHR surfaceFormat{};

    if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat.format = VK_FORMAT_R8G8B8A8_SRGB;
        surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        surfaceFormat = surfaceFormats[0];
    }

    return surfaceFormat;
}

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
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE; // TODO: mindfulness

    VkSwapchainKHR swapchain{};
    VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));
    return swapchain;
}

VkRenderPass CreateRenderPass(VkDevice device, VkFormat format)
{
    VkAttachmentDescription attachments[1]{};
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[1]{};
    colorAttachments[0].attachment = 0;
    colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpasses[1]{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = GS_ARRAY_COUNT(colorAttachments);
    subpasses[0].pColorAttachments = colorAttachments;

    VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = GS_ARRAY_COUNT(attachments);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = GS_ARRAY_COUNT(subpasses);
    createInfo.pSubpasses = subpasses;

    VkRenderPass renderPass{};
    VK_CHECK_RESULT(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));
    return renderPass;
}

VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool{};
    VK_CHECK_RESULT(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
    return commandPool;
}

VkSemaphore gsCreateSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore semaphore{};
    VK_CHECK_RESULT(vkCreateSemaphore(device, &createInfo, nullptr, &semaphore));
    return semaphore;
}

int wWinMainInternal(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    ComHelper comHelper;

    GameSmith::Window* applicationWindow = GameSmith::Window::CreateApplicationWindow("GameSmith Application", 1920, 1080);

    if (!applicationWindow)
    {
        GS_ERROR("Failed to create application window.");
        return EXIT_FAILURE;
    }

    VkInstance instance = CreateInstance();
    GS_ASSERT(instance);

    VkSurfaceKHR surface = CreateSurface(instance, hInstance, (HWND)applicationWindow->GetNativeHandle());
    GS_ASSERT(surface);

    VkPhysicalDevice physicalDevice{};
    uint32_t graphicsQueueIndex{};
    ChoosePhysicalDevice(instance, surface, physicalDevice, graphicsQueueIndex);
    GS_ASSERT(physicalDevice);

    VkDevice device = CreateDevice(physicalDevice, graphicsQueueIndex);
    GS_ASSERT(device);

    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(physicalDevice, surface);
    GS_ASSERT(surfaceFormat.format != VK_FORMAT_UNDEFINED);

    VkRenderPass renderPass = CreateRenderPass(device, surfaceFormat.format);
    GS_ASSERT(renderPass);

    VkSwapchainKHR swapchain = CreateSwapchain(physicalDevice, surface, device, surfaceFormat);
    GS_ASSERT(swapchain);

    uint32_t swapchainImageCount{};
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));
    std::vector<VkImage> swapchainImages(swapchainImageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

    std::vector<VkImageView> swapchainImageViews(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]));
    }

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

    std::vector<VkFramebuffer> framebuffers(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageView attachments[1] = { swapchainImageViews[i] };
        VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = GS_ARRAY_COUNT(attachments);
        createInfo.pAttachments = attachments;
        createInfo.width = surfaceCaps.currentExtent.width;
        createInfo.height = surfaceCaps.currentExtent.height;
        createInfo.layers = 1;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers[i]));
    }

    VkCommandPool commandPool = CreateCommandPool(device, graphicsQueueIndex);
    GS_ASSERT(commandPool);

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapchainImageCount;

    std::vector<VkCommandBuffer> commandBuffers(swapchainImageCount);
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));

    std::vector<VkFence> fences(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK_RESULT(vkCreateFence(device, &createInfo, nullptr, &fences[i]));
    }

    VkSemaphore acquireCompleteSemaphore = gsCreateSemaphore(device);
    GS_ASSERT(acquireCompleteSemaphore);

    VkSemaphore renderingCompleteSemaphore = gsCreateSemaphore(device);
    GS_ASSERT(renderingCompleteSemaphore);

    VkQueue graphicsQueue{};
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    GS_ASSERT(graphicsQueue);

    uint32_t imageIndex = 0;

    while (applicationWindow->IsValid())
    {
        applicationWindow->PumpMessages();

        VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquireCompleteSemaphore, VK_NULL_HANDLE, &imageIndex));
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fences[imageIndex], VK_TRUE, UINT64_MAX));
        VK_CHECK_RESULT(vkResetFences(device, 1, &fences[imageIndex]));

        VkCommandBuffer commandBuffer = commandBuffers[imageIndex];
        VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        VkImageMemoryBarrier renderBeginBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        renderBeginBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderBeginBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        renderBeginBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        renderBeginBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        renderBeginBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        renderBeginBarrier.image = swapchainImages[imageIndex];
        renderBeginBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        renderBeginBarrier.subresourceRange.baseMipLevel = 0;
        renderBeginBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        renderBeginBarrier.subresourceRange.baseArrayLayer = 0;
        renderBeginBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &renderBeginBarrier);

        VkClearValue clearValues[1]{};
        clearValues[0].color = { 43.0f / 255.0f, 53.0f / 255.0f, 51.0f / 255.0f, 1.0f };

        VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers[imageIndex];
        renderPassBeginInfo.renderArea.extent = surfaceCaps.currentExtent;
        renderPassBeginInfo.clearValueCount = GS_ARRAY_COUNT(clearValues);
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdEndRenderPass(commandBuffer);

        VkImageMemoryBarrier presentBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.image = swapchainImages[imageIndex];
        presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentBarrier.subresourceRange.baseMipLevel = 0;
        presentBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        presentBarrier.subresourceRange.baseArrayLayer = 0;
        presentBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &presentBarrier);

        vkEndCommandBuffer(commandBuffer);

        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &acquireCompleteSemaphore;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
        VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fences[imageIndex]));

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        VK_CHECK_RESULT(vkQueuePresentKHR(graphicsQueue, &presentInfo));
    }

    VK_CHECK_RESULT(vkDeviceWaitIdle(device));

    vkDestroySemaphore(device, renderingCompleteSemaphore, nullptr);
    vkDestroySemaphore(device, acquireCompleteSemaphore, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkDestroyFence(device, fences[i], nullptr);
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    return EXIT_SUCCESS;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    int result;

    if (IsDebuggerPresent())
    {
        result = wWinMainInternal(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
    }
    else
    {
        __try
        {
            result = wWinMainInternal(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
        }
        __except (UnhandledExceptionFilter(GetExceptionInformation()))
        {
            MessageBoxW(0, L"An exception occurred. The application will exit.", L"Unhandled exception", MB_ICONERROR | MB_OK);
            result = -1;
        }
    }

    return result;
}
