//
// Created by felix on 15/04/2021.
//

#ifndef G2_SHADER_H
#define G2_SHADER_H

#include <optional>
#include <span>

#include <vulkan/vulkan.h>

namespace g2::gfx {

VkShaderModule createShaderModule(VkDevice device, std::span<const uint32_t> code);

}

#endif  // G2_SHADER_H
