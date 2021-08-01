//
// Created by felix on 01/08/2021.
//

#ifndef G2_RENDERGRAPH_BUILDER_H
#define G2_RENDERGRAPH_BUILDER_H

#include "renderpass.h"
#include <vector>

namespace g2::gfx {



    class RendergraphBuilder {
    public:
        class PassBuilder {
            friend class RendergraphBuilder;
            const char* name;
            std::vector<AttachmentInfo> colorAttachments;
            std::optional<AttachmentInfo> depthAttachment;
            std::vector<ImageInputInfo> imageInputs;
        public:
            PassBuilder& color(const AttachmentInfo& a) {
                colorAttachments.push_back(a);
                return *this;
            };

            PassBuilder& depth(const AttachmentInfo& a) {
                depthAttachment = a;
                return *this;
            }

            PassBuilder& imageRead(uint32_t image) {
                imageInputs.push_back({
                    .image = image,
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                });
                return *this;
            }
        };

    private:
        std::vector<ImageInfo> images;
        std::vector<PassBuilder> renderPasseBuilders;
    public:

        uint32_t addImage(const ImageInfo& image) {
            images.push_back(image);
            return images.size() - 1;
        }

        PassBuilder& pass(const char* name) {
            renderPasseBuilders.emplace_back();
            renderPasseBuilders.back().name = name;
            return renderPasseBuilders.back();
        }

        RenderGraph* build(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews, uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {

            std::vector<RenderPassInfo> renderPasses;
            renderPasses.reserve(renderPasseBuilders.size());
            for(auto& b : renderPasseBuilders) {
                renderPasses.push_back(RenderPassInfo{
                    .name = b.name,
                    .colorAttachments = b.colorAttachments,
                    .depthAttachment = b.depthAttachment,
                    .imageInputs = b.imageInputs,
                });
            }

            RenderGraphInfo renderGraphInfo {
                .images = images,
                .renderPasses = renderPasses,
                .displayImages = displayViews,
                .displayWidth = displayWidth,
                .displayHeight = displayHeight,
                .displayFormat = displayFormat,
            };

            return createRenderGraph(device, allocator, &renderGraphInfo);
        }

    };


}

#endif //G2_RENDERGRAPH_BUILDER_H
