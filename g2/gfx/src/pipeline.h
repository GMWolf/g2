//
// Created by felix on 15/04/2021.
//

#ifndef G2_PIPELINE_H
#define G2_PIPELINE_H

#include <vulkan/vulkan.h>
#include "g2/gfx/pipeline_generated.h"
#include <g2/assets/asset_registry.h>

namespace g2::gfx {


    VkPipeline createPipeline(VkDevice device, const PipelineDef* pipelineDef, VkPipelineLayout pipelineLayout, VkFormat displayFormat);

    struct PipelineAssetManager : public IAssetManager {

        //TODO remove these members somehow
        VkDevice device;
        VkPipelineLayout layout;
        VkFormat displayFormat;

        std::vector<VkPipeline> pipelines;

        AssetAddResult add_asset(std::span<const char> data) override;

        const char *ext() override;
    };

}  // namespace g2::gfx
#endif  // G2_PIPELINE_H
