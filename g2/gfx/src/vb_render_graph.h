//
// Created by felix on 12/06/2021.
//

#ifndef G2_VB_RENDER_GRAPH_H
#define G2_VB_RENDER_GRAPH_H

#include "renderpass.h"
#include "rendergraph_builder.h"
#include <iostream>



namespace g2::gfx {

    static void materialDepth(RenderContext* ctx) {
        auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("materialDepth")];
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(ctx->cmd, 3, 1, 0, 0);
    }

    static void submitDrawItems(RenderContext* ctx) {
        vkCmdBindIndexBuffer(ctx->cmd,ctx->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        uint32_t itemIndex = 0;
        uint32_t drawIndex = 0;

        for(DrawItem& item : ctx->drawItems) {
            Mesh& mesh = ctx->meshManager->meshes[item.mesh];
            for(Primitive& prim : mesh.primitives) {
                for(Meshlet& meshlet : prim.meshlets) {
                    if (!meshletInView(ctx->cameraCullData, meshlet, ctx->transforms[itemIndex]))
                        continue;

                    ctx->transformMap[drawIndex] = ctx->transforms[itemIndex];

                    ctx->drawDataMap[drawIndex] = {
                            .baseIndex = static_cast<uint32_t>(prim.baseIndex + meshlet.triangleOffset),
                            .positionOffset = static_cast<uint32_t>(prim.positionOffset + meshlet.vertexOffset * 3),
                            .normalOffset = static_cast<uint32_t>(prim.normalOffset + meshlet.vertexOffset),
                            .texcoordOffset = static_cast<uint32_t>(prim.texcoordOffset + meshlet.vertexOffset),
                            .tangentOffset = static_cast<uint32_t>(prim.tangentOffset + meshlet.vertexOffset * 3),
                            .bitangentOffset = static_cast<uint32_t>(prim.bitangentOffset + meshlet.vertexOffset * 3),
                            .materialId = prim.material,
                            };

                    vkCmdPushConstants(ctx->cmd, ctx->pipelineLayout, VK_SHADER_STAGE_ALL, 0,
                                       sizeof(uint32_t), &drawIndex);
                    vkCmdDrawIndexed(ctx->cmd, meshlet.triangleCount * 3, 1, prim.baseIndex + meshlet.triangleOffset, 0, 0);

                    drawIndex++;
                }
            }

            itemIndex++;
        }

    }

    static void submitMaterialShade(RenderContext* ctx) {

        auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("visibility_debug")];
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        for(uint32_t matId = 0; matId < ctx->maxMaterialId; matId++) {

            vkCmdPushConstants(ctx->cmd, ctx->pipelineLayout, VK_SHADER_STAGE_ALL, 0,
                               sizeof(uint32_t), &matId);
            vkCmdDraw(ctx->cmd, 3, 1, 0, 0);
        }
    }

    static RenderGraph *createRenderGraph_vb(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews,
                                             uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {

        RendergraphBuilder graph;


        uint32_t shadowImage = graph.addImage({
            .size = {SHADOWMAP_SIZE, SHADOWMAP_SIZE},
            .format = VK_FORMAT_D32_SFLOAT,
            .binding = 4,
            });

        graph.pass("shadow")
            .callback([](RenderContext* ctx) {
                auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("shadow")];
                vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                submitDrawItems(ctx);
            })
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
                .callback([](RenderContext* ctx) {
                    auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("visibility")];
                    vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                    submitDrawItems(ctx);
                })
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
            .callback(materialDepth)
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
        .callback(submitMaterialShade)
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
