//
// Created by felix on 15/04/2021.
//

#include "shader.h"

vk::ShaderModule g2::gfx::createShaderModule(vk::Device device, std::span<char> code)
{
    vk::ShaderModuleCreateInfo createInfo
    {
        .codeSize = code.size_bytes(),
        .pCode = reinterpret_cast<uint32_t*>(code.data()),
    };

    auto result = device.createShaderModule(createInfo);
    if (result.result != vk::Result::eSuccess)
    {
        return {};
    }

    return result.value;
}
