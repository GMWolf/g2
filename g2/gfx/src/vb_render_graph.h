//
// Created by felix on 12/06/2021.
//

#ifndef G2_VB_RENDER_GRAPH_H
#define G2_VB_RENDER_GRAPH_H

#include "renderpass.h"
#include "rendergraph_builder.h"


namespace g2::gfx {
    static RenderGraph *createRenderGraph_vb(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews,
                                             uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {

        RendergraphBuilder graph;


        uint32_t shadowImage = graph.addImage({
            .size = {SHADOWMAP_SIZE, SHADOWMAP_SIZE},
            .format = VK_FORMAT_D32_SFLOAT,
            .binding = 4,
            });

        graph.pass("shadow")
            .depth({
                .image = shadowImage,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .depthStencil = { 1.0f, 0},
                },
            });

        uint32_t primitiveIdImage = graph.addImage({
            .size = {displayWidth, displayHeight},
            .format = VK_FORMAT_R32_UINT,
            .binding = 5,
            });

        uint32_t depthImage = graph.addImage({
            .size = {displayWidth, displayHeight},
            .format = VK_FORMAT_D32_SFLOAT,
            });

        graph.pass("visibility")
            .color({
                .image = primitiveIdImage,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .color = {0.0f, 0.0f, 0.0f, 0.0f},
                },
            })
            .depth({
                .image = depthImage,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = {
                        .depthStencil = {1.0f, 0},
                },
            });

        uint32_t materialIdImage = graph.addImage({
            .size = {displayWidth, displayHeight},
            .format = VK_FORMAT_D32_SFLOAT,
            });

        graph.pass("materialDepth")
            .depth({
                .image = materialIdImage,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .depthStencil{1.0f, 0},
                },
            })
            .imageRead(primitiveIdImage);

        graph.pass("visibility_debug")
        .color({
            .image = UINT32_MAX,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                    .color = { 1.0f, 0.0f, 1.0f, 1.0f },
             },
        })
        .depth({
            .image = materialIdImage,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        })
        .imageRead(shadowImage)
        .imageRead(primitiveIdImage);

        return graph.build(device, allocator, displayViews, displayWidth, displayHeight, displayFormat);
    }

}

#endif //G2_VB_RENDER_GRAPH_H
