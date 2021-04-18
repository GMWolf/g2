//
// Created by felix on 15/04/2021.
//

#include "pipeline.h"
#include "shader.h"
#include <fstream>
#include "renderpass.h"

g2::gfx::Pipeline g2::gfx::createPipeline(VkDevice device, const PipelineDef* pipeline_def, VkFormat displayFormat) {


  std::ifstream vertexInput(pipeline_def->shader()->vertex()->c_str(), std::ios::binary);
  std::vector<char> vertexBytes((std::istreambuf_iterator<char>(vertexInput)),
                                (std::istreambuf_iterator<char>()));

  std::ifstream fragmentInput(pipeline_def->shader()->fragment()->c_str(), std::ios::binary);
  std::vector<char> fragmentBytes((std::istreambuf_iterator<char>(fragmentInput)),
                                (std::istreambuf_iterator<char>()));

  VkShaderModule vertexModule = createShaderModule(device, vertexBytes);
  VkShaderModule fragmentModule = createShaderModule(device, fragmentBytes);


  VkPipelineShaderStageCreateInfo vertShaderStage{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertexModule,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo fragShaderStage{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragmentModule,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStage,
                                                      fragShaderStage};

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
      .cullMode = VK_CULL_MODE_BACK_BIT,
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

  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = false,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask =
         VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT |
             VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
  };

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = false,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
  };

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamicStates,
  };

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };



  VkPipelineLayout  pipeline_layout;
  if(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipeline_layout)) {
    return {};
  }


  //Create compatibility renderpass
  std::vector<VkFormat> formats;
  for(const Attachment* attachment : *pipeline_def->attachments())
  {
    if (attachment->format() == Format::display) {
      formats.push_back(displayFormat);
    } else {
      formats.push_back(static_cast<VkFormat>(attachment->format()));
    }
  }

  VkRenderPass render_pass = createCompatibilityRenderPass(device, formats);


  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
      .layout = pipeline_layout,
      .renderPass = render_pass,
      .subpass = 0,
      .basePipelineHandle = {},
      .basePipelineIndex = -1,
  };


  VkPipeline pipeline;
  if(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline)) {
    return {};
  }

  vkDestroyShaderModule(device, vertexModule, nullptr);
  vkDestroyShaderModule(device, fragmentModule, nullptr);
  vkDestroyRenderPass(device, render_pass, nullptr);

  return Pipeline {
      .pipeline = pipeline,
      .pipelineLayout = pipeline_layout,
  };
}
