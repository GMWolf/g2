//
// Created by felix on 15/04/2021.
//

#ifndef G2_PIPELINE_H
#define G2_PIPELINE_H

#include "vk.h"

namespace g2::gfx {

struct Pipeline {
  vk::Pipeline pipeline{};
  vk::PipelineLayout pipelineLayout{};

  inline operator bool() const { return pipeline; }
};

Pipeline createPipeline(vk::Device device, vk::ShaderModule vertex,
                        vk::ShaderModule fragment, vk::RenderPass renderPass,
                        uint32_t subpass);
}  // namespace g2::gfx
#endif  // G2_PIPELINE_H
