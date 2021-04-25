//
// Created by felix on 17/04/2021.
//

#include <g2/command_encoder.h>

#include "pipeline.h"
#include <vulkan/vulkan.h>
#include "mesh.h"

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

void g2::gfx::CommandEncoder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstindex,
                                          int32_t vertexOffset, uint32_t firstinstance) {
    auto cmd = reinterpret_cast<VkCommandBuffer>(cmdbuf);
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstindex, vertexOffset, firstinstance);
}

void g2::gfx::CommandEncoder::draw(const Mesh *mesh) {
    auto cmd = reinterpret_cast<VkCommandBuffer>(cmdbuf);
    for(auto& prim : mesh->primitives) {
        vkCmdDrawIndexed(cmd, prim.indexCount, 1, prim.baseIndex, 0, 0);
    }
}

