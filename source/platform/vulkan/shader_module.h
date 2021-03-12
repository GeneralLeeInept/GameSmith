#pragma once

#include "gsvulkan.h"

namespace gs
{
namespace vk
{

struct ShaderModule
{
    VkShaderModule shader;
    VkShaderStageFlagBits stage;
    std::string entryPoint;
};

ShaderModule loadShaderModule(VkDevice device, const std::string& path);

} // namespace Vulkan
} // namespace GameSmith
