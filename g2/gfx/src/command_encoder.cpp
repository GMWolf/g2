//
// Created by felix on 17/04/2021.
//

#include <g2/command_encoder.h>

#include "pipeline.h"
#include <vulkan/vulkan.h>

static_assert(sizeof(g2::gfx::CommandEncoder::cmdbuf) == sizeof(VkCommandBuffer));

void g2::gfx::CommandEncoder::bind_pipeline(VkPipeline pipeline) {
  auto cmd = reinterpret_cast<VkCommandBuffer>(cmdbuf);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void g2::gfx::CommandEncoder::draw(uint32_t vertexCount, uint32_t instanceCount,
                                  uint32_t firstVertex,
                                  uint32_t firstInstance) {
  auto cmd = reinterpret_cast<VkCommandBuffer>(cmdbuf);
  vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

