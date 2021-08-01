//
// Created by felix on 15/04/2021.
//

#include "pipeline.h"
#include "shader.h"
#include <fstream>
#include "renderpass.h"

VkPipeline g2::gfx::createPipeline(VkDevice device, const PipelineDef *pipeline_def, VkPipelineLayout pipelineLayout,
                                   VkFormat displayFormat) {


    size_t moduleCount = pipeline_def->modules()->size();

    const size_t maxModules = 5;
    assert(moduleCount <= maxModules);

    VkShaderModule modules[maxModules];
    for (int i = 0; i < pipeline_def->modules()->size(); i++) {
        auto text = pipeline_def->modules()->Get(i)->text();

        modules[i] = createShaderModule(device, std::span(text->data(), text->size()));
    }

    VkPipelineShaderStageCreateInfo shaderStages[maxModules];

    for (int i = 0; i < moduleCount; i++) {
        shaderStages[i] = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = static_cast<VkShaderStageFlagBits>(pipeline_def->modules()->Get(i)->stage()),
                .module = modules[i],
                .pName = "main",
        };
    }


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = false};

    // A dummy viewport and scissor. this needs to get set by cmd buffer
    VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = 1.0f,
            .height = 1.0f,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };

    VkRect2D scissor{
            .offset = {0, 0},
            .extent = {1, 1},
    };

    VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = static_cast<VkCullModeFlags>(pipeline_def->cullMode()),
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = false,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples =VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = false,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = false,
            .alphaToOneEnable = false,
    };

    const size_t maxBlendAttachments = 10;
    assert(pipeline_def->blending()->size() < maxBlendAttachments);
    VkPipelineColorBlendAttachmentState blendAttachments[maxBlendAttachments];

    for (int i = 0; i < pipeline_def->blending()->size(); i++) {
        auto blend = pipeline_def->blending()->Get(i);
        blendAttachments[i] = VkPipelineColorBlendAttachmentState{
                .blendEnable = blend->blend_enable(),
                .srcColorBlendFactor =
                static_cast<VkBlendFactor>(blend->src_blend_factor()),
                .dstColorBlendFactor =
                static_cast<VkBlendFactor>(blend->dst_blend_factor()),
                .colorBlendOp = static_cast<VkBlendOp>(blend->color_blend_op()),
                .srcAlphaBlendFactor =
                static_cast<VkBlendFactor>(blend->src_alpha_factor()),
                .dstAlphaBlendFactor =
                static_cast<VkBlendFactor>(blend->dst_alpha_factor()),
                .alphaBlendOp = static_cast<VkBlendOp>(blend->alpha_blend_op()),
                .colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT |
                                  VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
        };
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = false,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = pipeline_def->blending()->size(),
            .pAttachments = blendAttachments,
    };

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates,
    };


    //Create compatibility renderpass
    std::vector<VkFormat> formats;
    for (const Attachment *attachment : *pipeline_def->attachments()) {
        if (attachment->format() == Format::display) {
            formats.push_back(displayFormat);
        } else {
            formats.push_back(static_cast<VkFormat>(attachment->format()));
        }
    }

    VkFormat depthFormat =  pipeline_def->depthAttachment() ?
            static_cast<VkFormat>(pipeline_def->depthAttachment()->format())
            : VK_FORMAT_UNDEFINED;

    VkRenderPass render_pass = createCompatibilityRenderPass(device, formats, depthFormat);

    VkPipelineDepthStencilStateCreateInfo depthStencil {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = static_cast<VkCompareOp>(pipeline_def->depthCompare()),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(moduleCount),
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout,
            .renderPass = render_pass,
            .subpass = 0,
            .basePipelineHandle = {},
            .basePipelineIndex = -1,
    };


    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline)) {
        return {};
    }


    for (int i = 0; i < moduleCount; i++) {
        vkDestroyShaderModule(device, modules[i], nullptr);
    }

    vkDestroyRenderPass(device, render_pass, nullptr);

    return pipeline;
}

g2::AssetAddResult g2::gfx::PipelineAssetManager::add_asset(std::span<const char> data) {
    auto pipelineDef = flatbuffers::GetRoot<PipelineDef>(data.data());

    pipelines.push_back(createPipeline(device, pipelineDef, layout, displayFormat));

    return AssetAddResult{
            .index = static_cast<uint32_t>(pipelines.size() - 1),
            .patches = {},
    };
}

const char *g2::gfx::PipelineAssetManager::ext() {
    return ".g2ppln";
}
