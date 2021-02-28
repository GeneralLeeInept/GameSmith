#pragma once

#include <vulkan/vulkan.h>

namespace GameSmith
{
namespace Vulkan
{

struct ShaderModule
{
    VkShaderModule shader;
    VkShaderStageFlagBits stage;
    std::string entryPoint;
};

ShaderModule LoadShaderModule(VkDevice device, const std::string& path);

} // namespace Vulkan
} // namespace GameSmith
