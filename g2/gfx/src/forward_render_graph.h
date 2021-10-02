//
// Created by felix on 12/06/2021.
//

#ifndef G2_FORWARD_RENDER_GRAPH_H
#define G2_FORWARD_RENDER_GRAPH_H

#include "renderpass.h"
#include "imgui_impl_vulkan.h"



namespace g2::gfx {

    static void submitGeo(RenderContext* ctx) {
        vkCmdBindIndexBuffer(ctx->cmd,ctx->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        uint32_t itemIndex = 0;
        uint32_t drawIndex = 0;

        for(DrawItem& item : ctx->drawItems) {
            Mesh& mesh = ctx->meshManager->meshes[item.mesh];
            for(Primitive& prim : mesh.primitives) {
                for(Meshlet& meshlet : prim.meshlets) {
                    //if (!meshletInView(ctx->cameraCullData, meshlet, ctx->transforms[itemIndex]))
                    //    continue;

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

    static void submitDrawItems_forward(RenderContext* ctx) {
        auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("opaque")];
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        submitGeo(ctx);
    }

    static void submitDrawItems_shadow(RenderContext* ctx) {
        auto pipeline = ctx->pipelines[ctx->effect->getPipelineIndex("shadow")];
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        submitGeo(ctx);
    }

    static void imgui_callback_f(RenderContext* ctx)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx->cmd);
    }

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

        AttachmentInfo guiColorAttachments[] = {
                {   // Display attachment
                        .image = UINT32_MAX,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
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
                        .callback = submitDrawItems_shadow,
                },
                {
                        .name = "opaque",
                        .colorAttachments = colorAttachments,
                        .depthAttachment = depthAttachments,
                        .imageInputs = imageInputs,
                        .callback = submitDrawItems_forward,
                },
                {
                    .name = "imgui",
                    .colorAttachments = guiColorAttachments,
                    .depthAttachment = {},
                    .imageInputs = {},
                    .callback = imgui_callback_f,
                },
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
