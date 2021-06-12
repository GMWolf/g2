//
// Created by felix on 12/06/2021.
//

#ifndef G2_FORWARD_RENDER_GRAPH_H
#define G2_FORWARD_RENDER_GRAPH_H

#include "renderpass.h"



namespace g2::gfx {

    static RenderGraph *createRenderGraph_forward(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews,
                                                  uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {



        ImageInfo images[] = {
                {
                        .size = {displayWidth, displayHeight},
                        .format = VK_FORMAT_D32_SFLOAT,
                },
                {
                        .size = {SHADOWMAP_SIZE, SHADOWMAP_SIZE},
                        .format = VK_FORMAT_D32_SFLOAT,
                        .binding = 4,
                }
        };

        AttachmentInfo colorAttachments[] = {
                {   // Display attachment
                        .image = UINT32_MAX,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {
                                .color = {0.0f, 0.0f, 0.0f, 1.0f},
                        },
                },
        };

        AttachmentInfo depthAttachments = {
                .image = 0,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = {
                        .depthStencil = {1.0f, 0},
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
                }
        };

        RenderPassInfo renderPasses[] = {
                {
                        .name = "shadow",
                        .colorAttachments = {},
                        .depthAttachment = shadowAttachment,
                        .imageInputs = {},
                },
                {
                        .name = "opaque",
                        .colorAttachments = colorAttachments,
                        .depthAttachment = depthAttachments,
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

#endif //G2_FORWARD_RENDER_GRAPH_H
