//
// Created by felix on 15/04/2021.
//

#ifndef G2_PIPELINE_H
#define G2_PIPELINE_H

#include "volk.h"
#include "g2/gfx/pipeline_generated.h"

namespace g2::gfx {

struct Pipeline {
  VkPipeline pipeline{};
  VkPipelineLayout pipelineLayout{};

  inline operator bool() const { return pipeline; }
};

Pipeline createPipeline(VkDevice device, const PipelineDef* pipelineDef, VkFormat displayFormat);
}  // namespace g2::gfx
#endif  // G2_PIPELINE_H
