//
// Created by felix on 15/04/2021.
//

#ifndef G2_RENDERPASS_H
#define G2_RENDERPASS_H

#include "vk.h"
#include <span>

namespace g2::gfx {
vk::RenderPass createRenderPass(vk::Device device, vk::Format imageFormat);

vk::RenderPass createCompatibilityRenderPass(vk::Device device, std::span<vk::Format> imageFormats);

}

#endif  // G2_RENDERPASS_H
