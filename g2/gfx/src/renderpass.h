//
// Created by felix on 15/04/2021.
//

#ifndef G2_RENDERPASS_H
#define G2_RENDERPASS_H

#include "vk.h"
namespace g2::gfx {
vk::RenderPass createRenderPass(vk::Device device, vk::Format imageFormat);
}

#endif  // G2_RENDERPASS_H
