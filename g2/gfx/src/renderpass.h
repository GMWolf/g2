//
// Created by felix on 15/04/2021.
//

#ifndef G2_RENDERPASS_H
#define G2_RENDERPASS_H

#include <vulkan/vulkan.h>
#include <span>

namespace g2::gfx {
VkRenderPass createRenderPass(VkDevice device, VkFormat imageFormat);

VkRenderPass createCompatibilityRenderPass(VkDevice device, std::span<VkFormat> imageFormats);

}

#endif  // G2_RENDERPASS_H
