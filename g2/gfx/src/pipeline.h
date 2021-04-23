//
// Created by felix on 15/04/2021.
//

#ifndef G2_PIPELINE_H
#define G2_PIPELINE_H

#include <vulkan/vulkan.h>
#include "g2/gfx/pipeline_generated.h"

namespace g2::gfx {


VkPipeline createPipeline(VkDevice device, const PipelineDef* pipelineDef, VkPipelineLayout pipelineLayout, VkFormat displayFormat);
}  // namespace g2::gfx
#endif  // G2_PIPELINE_H
