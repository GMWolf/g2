//
// Created by felix on 15/04/2021.
//

#include "shader.h"

VkShaderModule g2::gfx::createShaderModule(VkDevice device, std::span<const uint32_t> code) {
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size_bytes(),
      .pCode = code.data(),
  };

  VkShaderModule shader_module;
  if(vkCreateShaderModule(device, &createInfo, nullptr, &shader_module) != VK_SUCCESS){
    return VK_NULL_HANDLE;
  }

  return shader_module;
}
