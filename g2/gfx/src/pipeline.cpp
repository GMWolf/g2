//
// Created by felix on 15/04/2021.
//

#include "pipeline.h"

g2::gfx::Pipeline g2::gfx::createPipeline(vk::Device device, vk::ShaderModule vertex, vk::ShaderModule fragment, vk::RenderPass renderPass, uint32_t subpass)
{
    vk::PipelineShaderStageCreateInfo vertShaderStage
    {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vertex,
        .pName = "main",
    };

    vk::PipelineShaderStageCreateInfo fragShaderStage
    {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = fragment,
        .pName = "main",
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStage, fragShaderStage };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly
    {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = false
    };

    // A dummy viewport and scissor. this needs to get set by cmd buffer
    vk::Viewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = 1.0f,
        .height = 1.0f,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vk::Rect2D scissor
    {
        .offset = {0,0},
        .extent = {1,1},
    };

    vk::PipelineViewportStateCreateInfo viewportState
    {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };


    vk::PipelineRasterizationStateCreateInfo rasterizer
    {
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = false,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    vk::PipelineMultisampleStateCreateInfo multisampling
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = false,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable = false,
    };


    vk::PipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = false,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eA
                | vk::ColorComponentFlagBits::eR
                | vk::ColorComponentFlagBits::eG
                | vk::ColorComponentFlagBits::eB,
    };


    vk::PipelineColorBlendStateCreateInfo colorBlending
    {
        .logicOpEnable = false,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    vk::PipelineDynamicStateCreateInfo dynamicState
    {
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    auto pipelineLayoutResult = device.createPipelineLayout(pipelineLayoutCreateInfo);
    if (pipelineLayoutResult.result != vk::Result::eSuccess)
    {
        return {};
    }

    vk::PipelineLayout pipelineLayout = pipelineLayoutResult.value;

    vk::GraphicsPipelineCreateInfo pipelineInfo
    {
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = subpass,
        .basePipelineHandle = {},
        .basePipelineIndex = -1,
    };

    auto pipelineResult = device.createGraphicsPipeline({}, pipelineInfo);

    if (pipelineResult.result != vk::Result::eSuccess)
    {
        return {};
    }

    return Pipeline
    {
        .pipeline = pipelineResult.value,
        .pipelineLayout = pipelineLayout,
    };

}
