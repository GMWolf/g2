//
// Created by felix on 12/06/2021.
//

#ifndef G2_VB_RENDER_GRAPH_H
#define G2_VB_RENDER_GRAPH_H

#include "renderpass.h"



namespace g2::gfx {

    static RenderGraph *createRenderGraph_vb(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews,
                                                  uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {

        ImageInfo images[] = {
                {
                        // depth
                        .size = {displayWidth, displayHeight},
                        .format = VK_FORMAT_D32_SFLOAT,
                },
                {
                        // shadow
                        .size = {SHADOWMAP_SIZE, SHADOWMAP_SIZE},
                        .format = VK_FORMAT_D32_SFLOAT,
                        .binding = 4,
                },
                {
                    // prim id
                    .size = { displayWidth, displayHeight },
                    .format = VK_FORMAT_R32_UINT,
                    .binding = 5,
                },
                {
                    // mat id
                    .size = { displayWidth, displayHeight },
                    .format = VK_FORMAT_D32_SFLOAT,
                }
        };



        AttachmentInfo shadowAttachment = {
                .image = 1,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .depthStencil = {1.0f, 0},
                }
        };

        ImageInputInfo imageInputs[] = {
                {
                        .image = 1,
                        .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },
                {
                    .image = 2,
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }
        };

        AttachmentInfo visPassColorAttachments[] = {
                {
                        .image = 2,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {
                                .color = {0.0f, 0.0f, 0.0f, 1.0f},
                        },
                }
        };

        AttachmentInfo depthAttachment = {
                .image = 0,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = {
                        .depthStencil = {1.0f, 0},
                }
        };

        AttachmentInfo displayAttachments[] = {
                {   // Display attachment
                        .image = UINT32_MAX,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {
                                .color = {0.0f, 0.0f, 0.0f, 1.0f},
                        },
                },
        };

        AttachmentInfo materialDepthWriteAttachment {
            .image = 3,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                    .depthStencil = {0.0f, 0},
            }
        };

        AttachmentInfo materialDepthReadAttachment {
            .image = 3,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
        };

        RenderPassInfo renderPasses[] = {
                {
                        .name = "shadow",
                        .colorAttachments = {},
                        .depthAttachment = shadowAttachment,
                        .imageInputs = {},
                },
                {
                        .name = "visibility",
                        .colorAttachments = visPassColorAttachments,
                        .depthAttachment = depthAttachment,
                        .imageInputs = imageInputs,
                },
                {
                    .name = "materialDepth",
                    .colorAttachments = {},
                    .depthAttachment = materialDepthWriteAttachment,
                },
                {
                    .name = "visibility_debug",
                    .colorAttachments = displayAttachments,
                    .depthAttachment = materialDepthReadAttachment,
                    .imageInputs = imageInputs,
                }
        };


        RenderGraphInfo renderGraphInfo{
                .images = images,
                .renderPasses = renderPasses,
                .displayImages = displayViews,
                .displayWidth = displayWidth,
                .displayHeight = displayHeight,
                .displayFormat = displayFormat,
        };

        return createRenderGraph(device, allocator, &renderGraphInfo);
    }
}

#endif //G2_VB_RENDER_GRAPH_H
