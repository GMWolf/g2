//
// Created by felix on 17/04/2021.
//

#include <g2/command_encoder.h>

#include "pipeline.h"
#include "vk.h"

static_assert(sizeof(g2::gfx::CommandEncoder::cmdbuf) == sizeof(vk::CommandBuffer));

void g2::gfx::CommandEncoder::bind_pipeline(const g2::gfx::Pipeline *pipeline) {
  vk::CommandBuffer& cmd = *reinterpret_cast<vk::CommandBuffer*>(&cmdbuf);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
}

void g2::gfx::CommandEncoder::draw(uint32_t vertexCount, uint32_t instanceCount,
                                  uint32_t firstVertex,
                                  uint32_t firstInstance) {
  vk::CommandBuffer& cmd = *reinterpret_cast<vk::CommandBuffer*>(&cmdbuf);
  cmd.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

