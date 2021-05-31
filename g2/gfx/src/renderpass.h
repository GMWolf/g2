//
// Created by felix on 15/04/2021.
//

#ifndef G2_RENDERPASS_H
#define G2_RENDERPASS_H

#include <vulkan/vulkan.h>
#include <span>
#include <optional>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

namespace g2::gfx {
    VkRenderPass createRenderPass(VkDevice device, VkFormat imageFormat);

    VkRenderPass createCompatibilityRenderPass(VkDevice device, std::span<VkFormat> imageFormats);


    struct ImageInfo {
        glm::uvec2 size;
        VkFormat format;
    };

    struct AttachmentInfo {
        uint32_t image;
        VkAttachmentLoadOp loadOp;
        VkAttachmentStoreOp storeOp;
        VkClearValue clearValue;
    };

    struct RenderPassInfo {
        std::span<AttachmentInfo> colorAttachments;
        std::optional<AttachmentInfo> depthAttachment;
    };

    struct RenderGraphInfo {
        std::span<ImageInfo> images;
        std::span<RenderPassInfo> renderPasses;

        std::span<VkImageView> displayImages;
        uint32_t displayWidth;
        uint32_t displayHeight;
        VkFormat displayFormat;

    };

    struct RenderGraph;

    RenderGraph* createRenderGraph(VkDevice device, VmaAllocator allocator, const RenderGraphInfo *renderGraphInfo);

    std::span<const VkRenderPassBeginInfo> getRenderPassInfos(const RenderGraph* renderGraph, uint32_t imageIndex);

}

#endif  // G2_RENDERPASS_H
