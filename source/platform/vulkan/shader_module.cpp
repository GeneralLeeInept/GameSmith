#include "gspch.h"

#include "shader_module.h"

#include "renderer_vk.h"
#include "gamesmith/core/debug.h"

#include <spirv-headers/spirv.h>

#define VK_CHECK_RESULT(call)            \
    do                                   \
    {                                    \
        VkResult result = (call);        \
        GS_ASSERT(result == VK_SUCCESS); \
    } while (0)

namespace gs
{
namespace vk
{

VkShaderStageFlagBits ShaderStage(SpvExecutionModel executionModel)
{
    switch (executionModel)
    {
        case SpvExecutionModelVertex:
        {
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
        case SpvExecutionModelFragment:
        {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        default:
        {
            GS_ASSERT(!"Unsupported execution model");
            return VkShaderStageFlagBits(0);
        }
    }
}

ShaderModule loadShaderModule(VkDevice device, const std::string& path)
{
    ShaderModule module{};

    std::ifstream fs(path, std::ios::in | std::ios::binary);
    GS_ASSERT(fs);
    auto fileSize = std::filesystem::file_size(path);
    std::vector<uint8_t> data(fileSize);
    fs.read((char*)data.data(), fileSize);

    uint32_t* code = (uint32_t*)data.data();
    uint32_t codeSize = (uint32_t)data.size();

    VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = codeSize;
    createInfo.pCode = code;

    VkShaderModule shaderModule{};
    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &module.shader));

    // Orientation / sanity checking; vkCreateShaderModule would probably have failed if any of this were untrue?
    GS_ASSERT(code[0] == SpvMagicNumber);
    GS_ASSERT(code[1] == SpvVersion);
    uint32_t boundIds = code[3];

    // Parse SPIR-V
    code = code + 5;
    codeSize -= 5 * sizeof(uint32_t);

    while (codeSize)
    {
        SpvOp opcode = SpvOp(code[0] & SpvOpCodeMask);
        uint32_t wordCount = code[0] >> SpvWordCountShift;

        switch (opcode)
        {
            case SpvOpEntryPoint:
            {
                SpvExecutionModel executionModel = SpvExecutionModel(code[1]);
                module.stage = ShaderStage(executionModel);
                module.entryPoint = (const char*)(&code[3]);
                break;
            }
        }

        code += wordCount;
        codeSize -= wordCount * 4;
    }

    // TODO: validate that we got everything we need here.

    return module;
}

} // namespace Vulkan
} // namespace GameSmith