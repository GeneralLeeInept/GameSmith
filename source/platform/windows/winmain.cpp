#include "gspch.h"

#include "gamesmith/core/core.h"
#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "gamesmith/core/window.h"
#include "gamesmith/math/math.h"
#include "gamesmith/math/mat44.h"
#include "gamesmith/math/vec3.h"
#include "gamesmith/renderer/obj_loader.h"
#include "platform/vulkan/gsvulkan.h"
#include "platform/vulkan/device.h"
#include "platform/vulkan/shader_module.h"
#include "platform/vulkan/swapchain.h"

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_vulkan.h>

#define ORTHO 0

struct ComHelper
{
    ComHelper() { CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); }
    ~ComHelper() { CoUninitialize(); }
};

struct Buffer
{
    VkBuffer buffer;
    VkDeviceMemory gpuMemory;
    VkDeviceSize size;
    void* mappedMemory;
};

struct Image
{
    VkImage image;
    VkDeviceMemory gpuMemory;
};

struct Framebuffer
{
    Image colorBuffer;
    Image depthBuffer;
    VkImageView colorBufferView;
    VkImageView depthBufferView;
    VkFramebuffer framebuffer;
};

struct MeshVertex
{
    float x, y, z;
    float nx, ny, nz;
};

struct Mesh
{
    uint32_t vertexCount;
    Buffer vertexBuffer;
    uint32_t indexCount;
    Buffer indexBuffer;
};

struct alignas(16) ShaderGlobals
{
    gs::mat44 proj;
    gs::mat44 view;
};

VkDebugUtilsMessengerEXT debugMessenger{};

VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    GS_WARN("%s", pCallbackData->pMessage);
    GS_ASSERT((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == 0);
    return VK_FALSE;
}

VkInstance createInstance()
{
    VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "GameSmith Application";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName = "GameSmith";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &applicationInfo;

    std::vector<const char*> enabledExtensions{ VK_KHR_SURFACE_EXTENSION_NAME };

#ifdef GS_PLATFORM_WINDOWS
    enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if GS_VULKAN_VALIDATION
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

#if GS_VULKAN_VALIDATION
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    debugCreateInfo.messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

    static const char* enabledLayers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = GS_ARRAY_COUNT(enabledLayers);
    createInfo.ppEnabledLayerNames = enabledLayers;
#endif

    VkInstance instance{};
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));

#if GS_VULKAN_VALIDATION
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    GS_ASSERT(CreateDebugUtilsMessengerEXT);

    debugCreateInfo.messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger));
#endif

    return instance;
}

VkSurfaceKHR createSurface(VkInstance instance, HINSTANCE hInstance, HWND hWnd)
{
    VkWin32SurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    createInfo.hinstance = hInstance;
    createInfo.hwnd = hWnd;

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
    return surface;
}

VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

void createFrameResources(VkDevice device, gs::vk::Swapchain& swapchain, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers,
                          std::vector<VkFence>& fences)
{
    uint32_t swapchainImageCount = uint32_t(swapchain.images.size());

    commandBuffers.resize(swapchainImageCount);

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapchainImageCount;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));

    fences.resize(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK_RESULT(vkCreateFence(device, &createInfo, nullptr, &fences[i]));
    }
}

void destroyFrameResources(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFence>& fences)
{
    for (VkFence& fence : fences)
    {
        vkDestroyFence(device, fence, nullptr);
    }

    fences.clear();

    vkFreeCommandBuffers(device, commandPool, uint32_t(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

VkRenderPass createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat)
{
    VkAttachmentDescription attachments[2]{};
    attachments[0].format = colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[1]{};
    colorAttachments[0].attachment = 0;
    colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachment{};
    depthStencilAttachment.attachment = 1;
    depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpasses[1]{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = GS_ARRAY_COUNT(colorAttachments);
    subpasses[0].pColorAttachments = colorAttachments;
    subpasses[0].pDepthStencilAttachment = &depthStencilAttachment;

    VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = GS_ARRAY_COUNT(attachments);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = GS_ARRAY_COUNT(subpasses);
    createInfo.pSubpasses = subpasses;

    VkRenderPass renderPass{};
    VK_CHECK_RESULT(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));
    return renderPass;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool{};
    VK_CHECK_RESULT(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
    return commandPool;
}

using ShaderList = std::initializer_list<gs::vk::ShaderModule>;

VkPipeline createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, ShaderList shaders)
{
    std::vector<VkPipelineShaderStageCreateInfo> stages{};

    for (auto& shader : shaders)
    {
        VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        createInfo.stage = shader.stage;
        createInfo.module = shader.shader;
        createInfo.pName = shader.entryPoint.c_str();
        stages.push_back(createInfo);
    }

    VkVertexInputBindingDescription vertexBindingDescriptions[1] = { { 0, sizeof(MeshVertex), VK_VERTEX_INPUT_RATE_VERTEX } };
    VkVertexInputAttributeDescription vertexAttributeDescriptions[2] = { { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
                                                                         { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12 } };

    VkPipelineVertexInputStateCreateInfo vertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputState.vertexBindingDescriptionCount = GS_ARRAY_COUNT(vertexBindingDescriptions);
    vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions;
    vertexInputState.vertexAttributeDescriptionCount = GS_ARRAY_COUNT(vertexAttributeDescriptions);
    vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER;

    VkPipelineColorBlendAttachmentState colorBlendStateAttachements[1]{};
    colorBlendStateAttachements[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendState.attachmentCount = GS_ARRAY_COUNT(colorBlendStateAttachements);
    colorBlendState.pAttachments = colorBlendStateAttachements;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = GS_ARRAY_COUNT(dynamicStates);
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    createInfo.stageCount = uint32_t(stages.size());
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = layout;
    createInfo.renderPass = renderPass;

    VkPipeline pipeline{};
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline));
    return pipeline;
}

VkSemaphore gsCreateSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore semaphore{};
    VK_CHECK_RESULT(vkCreateSemaphore(device, &createInfo, nullptr, &semaphore));
    return semaphore;
}

std::string gsWcharToUtf8(const wchar_t* wstring)
{
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
    std::string utf8;
    utf8.resize(utf8_len);
    WideCharToMultiByte(CP_UTF8, 0, wstring, -1, &utf8[0], utf8_len, NULL, NULL);
    return utf8;
}

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags requiredProperties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        VkMemoryType& memoryType = memoryProperties.memoryTypes[i];

        if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryType.propertyFlags & requiredProperties) == requiredProperties)
        {
            return i;
        }
    }

    return UINT32_MAX;
}

using QueueFamilyIndices = std::initializer_list<uint32_t>;

Buffer createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t size, VkBufferUsageFlags usage, QueueFamilyIndices queues)
{
    Buffer buffer{};

    std::vector<uint32_t> queueFamilyIndices{};

    for (auto& queueIndex : queues)
    {
        queueFamilyIndices.push_back(queueIndex);
    }

    VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = uint32_t(queueFamilyIndices.size());
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();

    VK_CHECK_RESULT(vkCreateBuffer(device, &createInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &memoryRequirements);
    buffer.size = memoryRequirements.size;

    VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex =
            getMemoryTypeIndex(physicalDevice, memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    GS_ASSERT(allocateInfo.memoryTypeIndex != UINT32_MAX);
    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, nullptr, &buffer.gpuMemory));
    VK_CHECK_RESULT(vkBindBufferMemory(device, buffer.buffer, buffer.gpuMemory, 0));
    VK_CHECK_RESULT(vkMapMemory(device, buffer.gpuMemory, 0, buffer.size, 0, &buffer.mappedMemory));

    return buffer;
}

void destroyBuffer(VkDevice device, Buffer& buffer)
{
    vkDestroyBuffer(device, buffer.buffer, nullptr);
    vkUnmapMemory(device, buffer.gpuMemory);
    vkFreeMemory(device, buffer.gpuMemory, nullptr);
    buffer = Buffer{};
}

void createImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImageCreateInfo& createInfo, Image& image)
{
    VK_CHECK_RESULT(vkCreateImage(device, &createInfo, nullptr, &image.image));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image.image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    GS_ASSERT(allocateInfo.memoryTypeIndex != UINT32_MAX);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, nullptr, &image.gpuMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, image.image, image.gpuMemory, 0));
}

void destroyImage(VkDevice device, Image& image)
{
    vkDestroyImage(device, image.image, nullptr);
    vkFreeMemory(device, image.gpuMemory, nullptr);
    image = Image{};
}

struct MeshVertexHasher
{
    static uint64_t floatbits(float f, uint8_t numbits)
    {
        uint32_t uf = *(uint32_t*)(&f);
        uint8_t shift = 23 - (numbits - 1);
        uint64_t r = 0ull;
        r |= (uf & 0x80000000) >> (31 - numbits);
        r |= (uf & 0x007fffff) >> (23 - numbits);
        return r;
    }

    uint64_t operator()(const MeshVertex& v) const
    {
        // 12-bits x, 12-bits y, 12-bits z, 10-bits nx, 9-bits ny, 9-bits nz
        uint64_t h = (floatbits(v.x, 12) << 52) | (floatbits(v.y, 12) << 40) | (floatbits(v.z, 12) << 28) | (floatbits(v.nx, 10) << 18) |
                     (floatbits(v.ny, 9) << 9) | (floatbits(v.nz, 9));

        return h;
    }
};

struct MeshVertexComparer
{
    bool operator()(const MeshVertex& lhs, const MeshVertex& rhs) const { return memcmp(&lhs, &rhs, sizeof(MeshVertex)) == 0; }
};

Mesh loadObjFile(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsQueueFamilyIndex, const std::string& path)
{
    gs::ObjFile objFile{};
    objFile.load(path);
    gs::ObjVertex* vertices = objFile.vertices.data();
    gs::ObjNormal* normals = objFile.normals.data();

    std::unordered_map<MeshVertex, size_t, MeshVertexHasher, MeshVertexComparer> vbdatalookup;
    std::vector<MeshVertex> vbdata{};
    std::vector<uint32_t> ibdata{};

    for (gs::ObjTri& tri : objFile.triangles)
    {
        for (int i = 0; i < 3; ++i)
        {
            MeshVertex v{};
            int32_t vi = tri.v[i];
            int32_t ni = tri.n[i];
            v.x = vertices[vi - 1].x;
            v.y = vertices[vi - 1].y;
            v.z = vertices[vi - 1].z;
            v.nx = (ni == -1) ? 0.f : normals[ni - 1].x;
            v.ny = (ni == -1) ? 0.f : normals[ni - 1].y;
            v.nz = (ni == -1) ? 1.f : normals[ni - 1].z;

            auto it = vbdatalookup.find(v);
            size_t index{};

            if (it == vbdatalookup.end())
            {
                index = vbdata.size();
                vbdatalookup.emplace(v, index);
                vbdata.push_back(v);
            }
            else
            {
                index = it->second;
            }

            ibdata.push_back(uint32_t(index));
        }
    }

    Mesh mesh{};

    mesh.vertexCount = uint32_t(vbdata.size());
    mesh.vertexBuffer = createBuffer(physicalDevice, device, mesh.vertexCount * sizeof(MeshVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                     { graphicsQueueFamilyIndex });
    memcpy(mesh.vertexBuffer.mappedMemory, vbdata.data(), vbdata.size() * sizeof(MeshVertex));

    mesh.indexCount = uint32_t(ibdata.size());
    mesh.indexBuffer =
            createBuffer(physicalDevice, device, mesh.indexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { graphicsQueueFamilyIndex });
    memcpy(mesh.indexBuffer.mappedMemory, ibdata.data(), ibdata.size() * sizeof(uint32_t));

    return mesh;
}

void destroyMesh(VkDevice device, Mesh& mesh)
{
    destroyBuffer(device, mesh.vertexBuffer);
    destroyBuffer(device, mesh.indexBuffer);
}

void createFramebuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkExtent2D extent, VkRenderPass renderPass, VkFormat colorFormat,
                       VkFormat depthFormat, Framebuffer& framebuffer)
{
    VkImageCreateInfo colorBufferImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    colorBufferImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    colorBufferImageCreateInfo.format = colorFormat;
    colorBufferImageCreateInfo.extent = { extent.width, extent.height, 1 };
    colorBufferImageCreateInfo.mipLevels = 1;
    colorBufferImageCreateInfo.arrayLayers = 1;
    colorBufferImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    colorBufferImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorBufferImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorBufferImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    colorBufferImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createImage(physicalDevice, device, colorBufferImageCreateInfo, framebuffer.colorBuffer);

    VkImageViewCreateInfo colorBufferImageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    colorBufferImageViewCreateInfo.image = framebuffer.colorBuffer.image;
    colorBufferImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorBufferImageViewCreateInfo.format = colorFormat;
    colorBufferImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBufferImageViewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    colorBufferImageViewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    VK_CHECK_RESULT(vkCreateImageView(device, &colorBufferImageViewCreateInfo, nullptr, &framebuffer.colorBufferView));

    VkImageCreateInfo depthBufferImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    depthBufferImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthBufferImageCreateInfo.format = depthFormat;
    depthBufferImageCreateInfo.extent = { extent.width, extent.height, 1 };
    depthBufferImageCreateInfo.mipLevels = 1;
    depthBufferImageCreateInfo.arrayLayers = 1;
    depthBufferImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthBufferImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthBufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBufferImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthBufferImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createImage(physicalDevice, device, depthBufferImageCreateInfo, framebuffer.depthBuffer);

    VkImageViewCreateInfo depthBufferImageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    depthBufferImageViewCreateInfo.image = framebuffer.depthBuffer.image;
    depthBufferImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthBufferImageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    depthBufferImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBufferImageViewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    depthBufferImageViewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthBufferImageViewCreateInfo, nullptr, &framebuffer.depthBufferView));

    VkImageView framebufferImageViews[2] = { framebuffer.colorBufferView, framebuffer.depthBufferView };
    VkFramebufferCreateInfo framebufferCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = framebufferImageViews;
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;
    VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer.framebuffer));
}

void destroyFramebuffer(VkDevice device, Framebuffer& framebuffer)
{
    vkDestroyFramebuffer(device, framebuffer.framebuffer, nullptr);
    vkDestroyImageView(device, framebuffer.depthBufferView, nullptr);
    vkDestroyImageView(device, framebuffer.colorBufferView, nullptr);
    destroyImage(device, framebuffer.depthBuffer);
    destroyImage(device, framebuffer.colorBuffer);
}

struct ImguiData
{
    VkDescriptorPool descriptorPool;
    VkRenderPass renderPass;
};

ImguiData initImgui(gs::Window* window, VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamily, VkQueue queue,
                    VkFormat colorBufferFormat, VkFormat depthBufferFormat, VkCommandBuffer commandBuffer)
{
    ImguiData result{};

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontDefault();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init((void*)window->getNativeHandle());

    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 } };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = GS_ARRAY_COUNT(poolSizes);
    descriptorPoolCreateInfo.pPoolSizes = poolSizes;
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &result.descriptorPool));

    VkAttachmentDescription attachments[2]{};
    attachments[0].format = colorBufferFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = depthBufferFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[1]{};
    colorAttachments[0].attachment = 0;
    colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachment{};
    depthStencilAttachment.attachment = 1;
    depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpasses[1]{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = GS_ARRAY_COUNT(colorAttachments);
    subpasses[0].pColorAttachments = colorAttachments;
    subpasses[0].pDepthStencilAttachment = &depthStencilAttachment;

    VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassCreateInfo.attachmentCount = GS_ARRAY_COUNT(attachments);
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = GS_ARRAY_COUNT(subpasses);
    renderPassCreateInfo.pSubpasses = subpasses;
    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &result.renderPass));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = queueFamily;
    initInfo.Queue = queue;
    //initInfo.PipelineCache;
    initInfo.DescriptorPool = result.descriptorPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2; // TODO: swapchain dependent?
    initInfo.ImageCount = 2;    // TODO:
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    //initInfo.Allocator;
    //initInfo.err;

    ImGui_ImplVulkan_Init(&initInfo, result.renderPass);

    // Upload font
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VK_CHECK_RESULT(vkDeviceWaitIdle(device));
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    return result;
}

void destroyImgui(VkDevice device, ImguiData& data)
{
    ImGui_ImplVulkan_Shutdown();
    vkDestroyRenderPass(device, data.renderPass, nullptr);
    vkDestroyDescriptorPool(device, data.descriptorPool, nullptr);
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

int wWinMainInternal(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    ComHelper comHelper;

    int argc;
    LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc);
    std::string objToLoad = gsWcharToUtf8(argv[1]);
    LocalFree(argv);

    gs::Window* applicationWindow = gs::Window::createApplicationWindow("GameSmith Application", 1024, 1024);

    if (!applicationWindow)
    {
        GS_ERROR("Failed to create application window.");
        return EXIT_FAILURE;
    }

    VkInstance instance = createInstance();
    GS_ASSERT(instance);

    VkSurfaceKHR surface = createSurface(instance, hInstance, (HWND)applicationWindow->getNativeHandle());
    GS_ASSERT(surface);

    VkPhysicalDevice physicalDevice{};
    uint32_t graphicsQueueIndex{};
    gs::vk::choosePhysicalDevice(instance, surface, physicalDevice, graphicsQueueIndex);
    GS_ASSERT(physicalDevice);

    VkDevice device = gs::vk::createDevice(physicalDevice, graphicsQueueIndex);
    GS_ASSERT(device);

    VkQueue graphicsQueue{};
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    GS_ASSERT(graphicsQueue);

    VkSemaphore acquireCompleteSemaphore = gsCreateSemaphore(device);
    GS_ASSERT(acquireCompleteSemaphore);

    VkSemaphore renderingCompleteSemaphore = gsCreateSemaphore(device);
    GS_ASSERT(renderingCompleteSemaphore);

    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(physicalDevice, surface);
    GS_ASSERT(surfaceFormat.format != VK_FORMAT_UNDEFINED);

    VkRenderPass renderPass = createRenderPass(device, surfaceFormat.format, VK_FORMAT_D32_SFLOAT);
    GS_ASSERT(renderPass);

    VkCommandPool commandPool = createCommandPool(device, graphicsQueueIndex);
    GS_ASSERT(commandPool);

    gs::vk::Swapchain swapchain{};
    gs::vk::createSwapchain(physicalDevice, device, surface, surfaceFormat, swapchain);
    GS_ASSERT(swapchain.swapchain);

    gs::vk::ShaderModule vertexShader = gs::vk::loadShaderModule(device, "triangle.vert.spv");
    gs::vk::ShaderModule fragmentShader = gs::vk::loadShaderModule(device, "triangle.frag.spv");

    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] = { { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL,
                                                                     nullptr } };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptorSetLayoutCreateInfo.bindingCount = GS_ARRAY_COUNT(descriptorSetLayoutBindings);
    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

    VkDescriptorSetLayout descriptorSetLayout{};
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCrateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutCrateInfo.setLayoutCount = 1;
    pipelineLayoutCrateInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout{};
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCrateInfo, nullptr, &pipelineLayout));

    VkPipeline pipeline = createGraphicsPipeline(device, renderPass, pipelineLayout, { vertexShader, fragmentShader });
    GS_ASSERT(pipeline);

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkFence> fences;
    createFrameResources(device, swapchain, commandPool, commandBuffers, fences);

    Mesh mesh = loadObjFile(physicalDevice, device, graphicsQueueIndex, objToLoad);

    // Create off-screen buffer for rendering
    Framebuffer framebuffer{};
    createFramebuffer(physicalDevice, device, swapchain.extent, renderPass, surfaceFormat.format, VK_FORMAT_D32_SFLOAT, framebuffer);
    GS_ASSERT(framebuffer.framebuffer);

    // Create descriptor pool
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 32 } };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolCreateInfo.maxSets = 32;
    descriptorPoolCreateInfo.poolSizeCount = GS_ARRAY_COUNT(poolSizes);
    descriptorPoolCreateInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool{};
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

    // Create globals UBO
    // TODO: dynamic UBO, needs to be at least swapchain image count * sizeof(ShaderGlobals), assume
    // swapchain image count is always <= 4 for now because dealing with resizing is gross at the moment
    Buffer globalsBuffer =
            createBuffer(physicalDevice, device, sizeof(ShaderGlobals) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, { graphicsQueueIndex });
    GS_ASSERT(globalsBuffer.buffer);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet{};
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

    VkDescriptorBufferInfo descriptorBufferInfo{};
    descriptorBufferInfo.buffer = globalsBuffer.buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = sizeof(ShaderGlobals);

    VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

    ImguiData imguiData = initImgui(applicationWindow, instance, physicalDevice, device, graphicsQueueIndex, graphicsQueue, surfaceFormat.format,
                                    VK_FORMAT_D32_SFLOAT, commandBuffers[0]);

    while (applicationWindow->isValid())
    {
        applicationWindow->pumpMessages();

        // IMGUI HACKING
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = float(cursor_pos.x);
        io.MousePos.y = float(cursor_pos.y);
        io.MouseDown[0] = GetKeyState(VK_LBUTTON) & 0x8000;
        io.MouseDown[1] = GetKeyState(VK_RBUTTON) & 0x8000;
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Begin("Another Window");
        ImGui::End();

        ImGui::Render();
        ////////////////

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

        if (memcmp(&swapchain.extent, &surfaceCaps.currentExtent, sizeof(VkExtent2D)) != 0)
        {
            VK_CHECK_RESULT(vkDeviceWaitIdle(device));
            destroySwapchain(device, swapchain);
            destroyFrameResources(device, commandPool, commandBuffers, fences);

            createSwapchain(physicalDevice, device, surface, surfaceFormat, swapchain);
            GS_ASSERT(swapchain.swapchain);
            createFrameResources(device, swapchain, commandPool, commandBuffers, fences);

            destroyFramebuffer(device, framebuffer);
            createFramebuffer(physicalDevice, device, swapchain.extent, renderPass, surfaceFormat.format, VK_FORMAT_D32_SFLOAT, framebuffer);
        }

        uint32_t imageIndex{};
        VkResult aniresult = vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX, acquireCompleteSemaphore, VK_NULL_HANDLE, &imageIndex);
        VK_CHECK_RESULT(aniresult);
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fences[imageIndex], VK_TRUE, UINT64_MAX));
        VK_CHECK_RESULT(vkResetFences(device, 1, &fences[imageIndex]));

        VkCommandBuffer commandBuffer = commandBuffers[imageIndex];
        VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        uint32_t dynamicOffset = imageIndex * sizeof(ShaderGlobals);
        float viewWidth = float(swapchain.extent.width);
        float viewHeight = float(swapchain.extent.height);
        float viewAspect = viewWidth / viewHeight;

#if ORTHO
        ((ShaderGlobals*)globalsBuffer.mappedMemory)[imageIndex].proj = gs::orthographic(-viewAspect, -1.f, viewAspect, 1.f, -1.f, 1.f);
        ((ShaderGlobals*)globalsBuffer.mappedMemory)[imageIndex].view = gs::mat44();
#else
        ((ShaderGlobals*)globalsBuffer.mappedMemory)[imageIndex].proj = gs::perspective(gs::degToRad(60.f), viewAspect, 0.1f, 100.f);
        ((ShaderGlobals*)globalsBuffer.mappedMemory)[imageIndex].view = gs::lookAt({ -1.f, 0.5f, 1.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f });
#endif

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);

        VkClearValue clearValues[2]{};
        clearValues[0].color = { 43.0f / 255.0f, 53.0f / 255.0f, 51.0f / 255.0f, 1.0f };
        clearValues[1].depthStencil = { 0.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffer.framebuffer;
        renderPassBeginInfo.renderArea.extent = swapchain.extent;
        renderPassBeginInfo.clearValueCount = GS_ARRAY_COUNT(clearValues);
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkViewport viewport{ 0.f, (float)swapchain.extent.height, (float)swapchain.extent.width, -(float)swapchain.extent.height, 1.f, 0.f };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = swapchain.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets{};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offsets);
        vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = imguiData.renderPass;
        renderPassBeginInfo.framebuffer = framebuffer.framebuffer;
        renderPassBeginInfo.renderArea.extent = swapchain.extent;
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        vkCmdEndRenderPass(commandBuffer);

        VkImageMemoryBarrier copyBarriers[2]{};
        copyBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copyBarriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        copyBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        copyBarriers[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        copyBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copyBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarriers[0].image = framebuffer.colorBuffer.image;
        copyBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyBarriers[0].subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        copyBarriers[0].subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        copyBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copyBarriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copyBarriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copyBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarriers[1].image = swapchain.images[imageIndex];
        copyBarriers[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyBarriers[1].subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        copyBarriers[1].subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, GS_ARRAY_COUNT(copyBarriers), copyBarriers);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.extent = { swapchain.extent.width, swapchain.extent.height, 1 };

        vkCmdCopyImage(commandBuffer, framebuffer.colorBuffer.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.images[imageIndex],
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        VkImageMemoryBarrier presentBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        presentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        presentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.image = swapchain.images[imageIndex];
        presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        presentBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0,
                             nullptr, 0, nullptr, 1, &presentBarrier);

        vkEndCommandBuffer(commandBuffer);

        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
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
        presentInfo.pSwapchains = &swapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;
        VK_CHECK_RESULT(vkQueuePresentKHR(graphicsQueue, &presentInfo));
    }

    VK_CHECK_RESULT(vkDeviceWaitIdle(device));

    destroyImgui(device, imguiData);

    destroyMesh(device, mesh);

    destroyBuffer(device, globalsBuffer);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    destroyFramebuffer(device, framebuffer);
    destroyFrameResources(device, commandPool, commandBuffers, fences);
    destroySwapchain(device, swapchain);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyShaderModule(device, fragmentShader.shader, nullptr);
    vkDestroyShaderModule(device, vertexShader.shader, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroySemaphore(device, renderingCompleteSemaphore, nullptr);
    vkDestroySemaphore(device, acquireCompleteSemaphore, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

#if GS_VULKAN_VALIDATION
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    GS_ASSERT(DestroyDebugUtilsMessengerEXT);
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

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
