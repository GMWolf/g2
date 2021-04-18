//
// Created by felix on 15/04/2021.
//

#include "shader.h"

VkShaderModule g2::gfx::createShaderModule(VkDevice device, std::span<char> code) {
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size_bytes(),
      .pCode = reinterpret_cast<uint32_t *>(code.data()),
  };

  VkShaderModule shader_module;
  if(vkCreateShaderModule(device, &createInfo, nullptr, &shader_module) != VK_SUCCESS){
    return VK_NULL_HANDLE;
  }

  return shader_module;
}
