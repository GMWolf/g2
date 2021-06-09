//
// Created by felix on 15/04/2021.
//

#include "renderpass.h"
#include <vector>

#include <iostream>

namespace g2::gfx {

    VkRenderPass createRenderPass(VkDevice device, VkFormat imageFormat) {
        VkAttachmentDescription colorAttachment{
                .format = imageFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference colorAttachmentRef{
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentDescription depthAttachment{
                .format = VK_FORMAT_D32_SFLOAT,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentRef{
                .attachment = 1,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentRef,
                .pDepthStencilAttachment = &depthAttachmentRef,
        };

        VkSubpassDependency dependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .srcAccessMask = {},
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

        VkAttachmentDescription attachments[2] = {
                colorAttachment, depthAttachment,
        };

        VkRenderPassCreateInfo renderPassCreateInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 2,
                .pAttachments = attachments,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency,
        };

        VkRenderPass render_pass;
        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &render_pass) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        return render_pass;
    }

    VkRenderPass
    createCompatibilityRenderPass(VkDevice device, std::span<VkFormat> imageFormats, VkFormat depthFormat) {

        std::vector<VkAttachmentDescription> attachments;
        attachments.reserve(imageFormats.size() + 1);

        for (VkFormat format : imageFormats) {
            VkAttachmentDescription attachment{
                    .format = format,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };

            attachments.push_back(attachment);
        }

        std::vector<VkAttachmentReference> colorAttachmentRefs;
        colorAttachmentRefs.reserve(attachments.size());
        for (uint32_t index = 0; index < attachments.size(); index++) {
            colorAttachmentRefs.push_back(
                    {
                            .attachment = index,
                            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    });
        };

        VkAttachmentDescription depthAttachment{
                .format = depthFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        attachments.push_back(depthAttachment);

        VkAttachmentReference depthAttachmentRef{
                .attachment = static_cast<uint32_t>(attachments.size() - 1),
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size()),
                .pColorAttachments = colorAttachmentRefs.data(),
                .pDepthStencilAttachment = &depthAttachmentRef,
        };


        VkRenderPassCreateInfo renderPassCreateInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 0,
                .pDependencies = nullptr,
        };

        VkRenderPass render_pass;
        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &render_pass) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        return render_pass;

    }

    struct PassTransitions {
        std::vector<VkImageLayout> colorAttachmentInitialLayouts;
        std::vector<VkImageLayout> colorAttachmentFinalLayouts;
        VkImageLayout depthAttachmentInitialLayout;
        VkImageLayout depthAttachmentFinalLayout;
    };

    struct Pass {
        std::string name;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> framebuffers;
        VkRect2D renderArea;
        std::vector<VkClearValue> clearValues;
    };

    struct RenderGraph {
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VmaAllocation> imageAllocations;

        std::vector<Pass> passes;

        std::vector<std::vector<PassInfo>> renderPassInfos;
        std::vector<ImageBinding> imageBindings;
    };

    static void getImageUsages(std::span<RenderPassInfo> passes, std::span<VkImageUsageFlags> outUsages) {
        for (auto pass : passes) {
            for (auto attachment : pass.colorAttachments) {
                if (attachment.image == UINT32_MAX) {
                    continue;
                }

                assert(attachment.image < outUsages.size());
                outUsages[attachment.image] |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            if ( pass.depthAttachment ) {
                assert(pass.depthAttachment->image < outUsages.size());
                outUsages[pass.depthAttachment->image] |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }

            for(auto input : pass.imageInputs) {
                outUsages[input.image] |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
        }
    }

    static void createImages(VkDevice device, VmaAllocator allocator, std::span<ImageInfo> imageInfos,
                             std::span<VkImageUsageFlags> usages,
                             std::span<VkImage> outImages, std::span<VmaAllocation> outAllocations,
                             std::span<VkImageView> views) {
        assert(imageInfos.size() == usages.size());

        for (size_t index = 0; index < imageInfos.size(); index++) {
            VkImageCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags = 0,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = imageInfos[index].format,
                    .extent = {
                            .width = imageInfos[index].size.x,
                            .height = imageInfos[index].size.y,
                            .depth = 1,
                    },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage = usages[index],
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };

            VmaAllocationCreateInfo allocInfo{
                    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            };

            vmaCreateImage(allocator, &createInfo, &allocInfo, &outImages[index], &outAllocations[index], nullptr);

            VkImageAspectFlags aspect = 0;
            if (imageInfos[index].format == VK_FORMAT_D32_SFLOAT) { //TODO all depth formats
                aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
            } else {
                aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
            };

            VkImageViewCreateInfo viewInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = outImages[index],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = imageInfos[index].format,
                    .components = VkComponentMapping{
                            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                            .aspectMask = aspect,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                    },
            };

            vkCreateImageView(device, &viewInfo, nullptr, &views[index]);
        }
    }

    void createRenderPass(VkDevice device, const RenderPassInfo *renderPassInfo,
                          std::span<ImageInfo> images, VkFormat displayFormat,
                          const PassTransitions* transitions,
                          VkRenderPass *outRenderPass) {

        std::vector<VkAttachmentDescription> attachments;
        attachments.reserve(renderPassInfo->colorAttachments.size() + 1);

        for(uint32_t index = 0; index < renderPassInfo->colorAttachments.size(); index++) {
            auto& attachmentInfo = renderPassInfo->colorAttachments[index];

            auto format = attachmentInfo.image == UINT32_MAX ? displayFormat : images[attachmentInfo.image].format;

            VkAttachmentDescription attachment{
                    .format = format,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = attachmentInfo.loadOp,
                    .storeOp = attachmentInfo.storeOp,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = transitions->colorAttachmentInitialLayouts[index],
                    .finalLayout = transitions->colorAttachmentFinalLayouts[index],
            };

            attachments.push_back(attachment);
        }

        std::vector<VkAttachmentReference> colourAttachmentRefs(renderPassInfo->colorAttachments.size());
        for (uint32_t index = 0; index < renderPassInfo->colorAttachments.size(); index++) {
            colourAttachmentRefs[index] = {
                    .attachment = index,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
        }

        std::optional<VkAttachmentReference> depthAttachmentRef;

        if (renderPassInfo->depthAttachment) {
            VkAttachmentDescription depthAttachment{
                    .format = images[renderPassInfo->depthAttachment->image].format,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = renderPassInfo->depthAttachment->loadOp,
                    .storeOp = renderPassInfo->depthAttachment->storeOp,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = transitions->depthAttachmentInitialLayout,
                    .finalLayout = transitions->depthAttachmentFinalLayout,
            };

            attachments.push_back(depthAttachment);

            depthAttachmentRef = VkAttachmentReference{
                    .attachment = static_cast<uint32_t>(attachments.size() - 1),
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

        }

        VkSubpassDescription subpass {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = static_cast<uint32_t>(colourAttachmentRefs.size()),
                .pColorAttachments = colourAttachmentRefs.data(),
                .pDepthStencilAttachment = depthAttachmentRef ? &depthAttachmentRef.value() : nullptr,
        };


        VkRenderPassCreateInfo renderPassCreateInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 0,
                .pDependencies = nullptr,
        };


        if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, outRenderPass) != VK_SUCCESS) {
            //TODO: handle error
        }

    }


    static void createFramebuffer(VkDevice device,
                                  const RenderPassInfo *renderPassInfo,
                                  std::span<VkImageView> imageViews,
                                  VkImageView displayView,
                                  VkRenderPass renderPass,
                                  uint32_t width,
                                  uint32_t height,
                                  VkFramebuffer *outFramebuffer) {

        std::vector<VkImageView> attachments;
        attachments.reserve(
                renderPassInfo->colorAttachments.size() + (renderPassInfo->depthAttachment.has_value() ? 1 : 0)
        );

        attachments.clear();
        for (auto &attachment : renderPassInfo->colorAttachments) {
            attachments.push_back(attachment.image == UINT32_MAX ? displayView : imageViews[attachment.image]);
        }
        if (renderPassInfo->depthAttachment) {
            attachments.push_back(imageViews[renderPassInfo->depthAttachment->image]);
        }

        VkFramebufferCreateInfo framebufferInfo {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = width,
                .height = height,
                .layers = 1,
        };

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, outFramebuffer) != VK_SUCCESS) {
            std::cerr << "Error create framerbuffer\n";
        }

    }

    static VkImageLayout getNextImageLayout(const RenderGraphInfo *renderGraphInfo, uint32_t imageIndex, uint32_t passIndex, VkImageLayout initial) {

        for(uint32_t pass = passIndex + 1; pass < renderGraphInfo->renderPasses.size(); pass++) {
            auto& colorAttachments = renderGraphInfo->renderPasses[pass].colorAttachments;
            for(uint32_t attachmentIndex = 0; attachmentIndex < colorAttachments.size(); attachmentIndex++) {
                if(colorAttachments[attachmentIndex].image == imageIndex) {
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
            }

            if(renderGraphInfo->renderPasses[pass].depthAttachment->image == imageIndex) {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            auto& inputs = renderGraphInfo->renderPasses[pass].imageInputs;
            for(uint32_t inputIndex = 0; inputIndex < inputs.size(); inputIndex++) {
                if(inputs[inputIndex].image == imageIndex) {
                    return inputs[inputIndex].layout;
                }
            }
        }

        return imageIndex == UINT32_MAX ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : initial;
    }

    static void createRenderPasses(VkDevice device, const RenderGraphInfo *renderGraphInfo,
                                   std::span<VkImageView> imageViews,
                                   std::span<VkImageView> displayViews,
                                   std::span<Pass> outPasses) {

        //Compute transitions
        std::vector<VkImageLayout> imageLayouts(imageViews.size(), VK_IMAGE_LAYOUT_UNDEFINED);
        VkImageLayout displayLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        std::vector<PassTransitions> passTransitions(renderGraphInfo->renderPasses.size());
        for(uint32_t passIndex = 0; passIndex < renderGraphInfo->renderPasses.size(); passIndex++) {
            auto& renderPassInfo = renderGraphInfo->renderPasses[passIndex];
            auto& transition = passTransitions[passIndex];
            transition.colorAttachmentInitialLayouts.resize(renderPassInfo.colorAttachments.size());
            transition.colorAttachmentFinalLayouts.resize(renderPassInfo.colorAttachments.size());
            for(uint32_t attachment = 0; attachment < renderPassInfo.colorAttachments.size(); attachment++) {
                auto imageIndex = renderPassInfo.colorAttachments[attachment].image;
                transition.colorAttachmentInitialLayouts[attachment] = imageIndex == UINT32_MAX ? displayLayout : imageLayouts[imageIndex];
                auto nextImageLayout = getNextImageLayout(renderGraphInfo, imageIndex, passIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                transition.colorAttachmentFinalLayouts[attachment] = nextImageLayout;
                if(imageIndex == UINT32_MAX) {
                    displayLayout = nextImageLayout;
                } else {
                    imageLayouts[imageIndex] = nextImageLayout;
                }
            }

            if(renderPassInfo.depthAttachment) {
                transition.depthAttachmentInitialLayout = imageLayouts[renderPassInfo.depthAttachment->image];
                auto nextImageLayout = getNextImageLayout(renderGraphInfo, renderPassInfo.depthAttachment->image, passIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                transition.depthAttachmentFinalLayout = nextImageLayout;
                imageLayouts[renderPassInfo.depthAttachment->image] = nextImageLayout;
            }

        }


        //Create passes
        for (uint32_t index = 0; index < renderGraphInfo->renderPasses.size(); index++) {
            auto renderPassInfo = &renderGraphInfo->renderPasses[index];
            createRenderPass(device, renderPassInfo, renderGraphInfo->images, renderGraphInfo->displayFormat,
                             &passTransitions[index], &outPasses[index].renderPass);

            outPasses[index].name = renderPassInfo->name;

            bool isDisplayRenderBuffer =
                    std::find_if(renderPassInfo->colorAttachments.begin(), renderPassInfo->colorAttachments.end(),
                                 [](const AttachmentInfo &attachmentInfo) {
                                     return attachmentInfo.image == UINT32_MAX;
                                 }) != renderPassInfo->colorAttachments.end();


            uint32_t width, height;

            if (isDisplayRenderBuffer) {
                width = renderGraphInfo->displayWidth;
                height = renderGraphInfo->displayHeight;

                outPasses[index].framebuffers.resize(displayViews.size());
                for (int i = 0; i < displayViews.size(); i++) {
                    createFramebuffer(device, renderPassInfo, imageViews, displayViews[i], outPasses[index].renderPass,
                                      width, height, &outPasses[index].framebuffers[i]);
                }


            } else {
                if (renderPassInfo->colorAttachments.empty()) {
                    width = renderGraphInfo->images[renderPassInfo->depthAttachment->image].size.x;
                    height = renderGraphInfo->images[renderPassInfo->depthAttachment->image].size.y;
                } else {
                    width = renderGraphInfo->images[renderPassInfo->colorAttachments[0].image].size.x;
                    height = renderGraphInfo->images[renderPassInfo->colorAttachments[0].image].size.y;
                }
                outPasses[index].framebuffers.resize(1);
                createFramebuffer(device, renderPassInfo, imageViews, VK_NULL_HANDLE, outPasses[index].renderPass,
                                  width, height, &outPasses[index].framebuffers[0]);
            }

            outPasses[index].renderArea = {
                    .offset = {0, 0},
                    .extent = {width, height},
            };

            for (auto attachment : renderPassInfo->colorAttachments) {
                outPasses[index].clearValues.push_back(attachment.clearValue);
            }
            if (renderPassInfo->depthAttachment) {
                outPasses[index].clearValues.push_back(renderPassInfo->depthAttachment->clearValue);
            }

        }
    }

    static void createRenderPassInfos(std::span<Pass> passes, std::span<PassInfo> outRenderPassInfo,
                                      int displayImageIndex) {

        for (int i = 0; i < passes.size(); i++) {
            outRenderPassInfo[i].name = passes[i].name.c_str();
            outRenderPassInfo[i].passBeginInfo = VkRenderPassBeginInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = passes[i].renderPass,
                    .framebuffer = passes[i].framebuffers.size() > 1 ? passes[i].framebuffers[displayImageIndex]
                                                                     : passes[i].framebuffers[0],
                    .renderArea = passes[i].renderArea,
                    .clearValueCount = static_cast<uint32_t>(passes[i].clearValues.size()),
                    .pClearValues = passes[i].clearValues.data(),
            };
        }

    }

    RenderGraph *createRenderGraph(VkDevice device, VmaAllocator allocator, const RenderGraphInfo *renderGraphInfo) {

        auto *renderGraph = new RenderGraph();
        renderGraph->images.resize(renderGraphInfo->images.size());
        renderGraph->imageViews.resize(renderGraphInfo->images.size());
        renderGraph->imageAllocations.resize(renderGraphInfo->images.size());

        std::vector<VkImageUsageFlags> imageUsages(renderGraphInfo->images.size());
        getImageUsages(renderGraphInfo->renderPasses, imageUsages);

        createImages(device, allocator, renderGraphInfo->images, imageUsages,
                     renderGraph->images, renderGraph->imageAllocations, renderGraph->imageViews);

        renderGraph->passes.resize(renderGraphInfo->renderPasses.size());

        createRenderPasses(device, renderGraphInfo, renderGraph->imageViews, renderGraphInfo->displayImages,
                           renderGraph->passes);


        renderGraph->renderPassInfos.resize(renderGraphInfo->displayImages.size());
        for (int i = 0; i < renderGraphInfo->displayImages.size(); i++) {
            renderGraph->renderPassInfos[i].resize(renderGraph->passes.size());
            createRenderPassInfos(renderGraph->passes, renderGraph->renderPassInfos[i], i);
        }

        renderGraph->imageBindings.reserve(renderGraphInfo->images.size());
        for(int i = 0; i < renderGraphInfo->images.size(); i++) {
            if(renderGraphInfo->images[i].binding) {
                renderGraph->imageBindings.push_back(ImageBinding{
                    .imageView = renderGraph->imageViews[i],
                    .binding = *renderGraphInfo->images[i].binding,
                });
            }
        }

        return renderGraph;
    }

    std::span<const PassInfo> getRenderPassInfos(const RenderGraph *renderGraph, uint32_t imageIndex) {
        return renderGraph->renderPassInfos[imageIndex];
    }


    std::span<const VkImageView> getImageViews(const RenderGraph* renderGraph) {
        return renderGraph->imageViews;
    }

    std::span<const ImageBinding> getImageBindings(const RenderGraph *renderGraph) {
        return renderGraph->imageBindings;
    }

    void destroyRenderGraph(VkDevice device, VmaAllocator allocator, RenderGraph *renderGraph) {

        for(auto pass : renderGraph->passes) {
            for(auto fb : pass.framebuffers) {
                vkDestroyFramebuffer(device, fb, nullptr);
            }
            vkDestroyRenderPass(device, pass.renderPass, nullptr);
        }
        renderGraph->passes.clear();

        for(auto view : renderGraph->imageViews) {
            vkDestroyImageView(device, view, nullptr);
        }
        renderGraph->imageViews.clear();

        for(size_t index = 0; index < renderGraph->images.size(); index++) {
            vmaDestroyImage(allocator, renderGraph->images[index], renderGraph->imageAllocations[index]);
        }
        renderGraph->images.clear();
        renderGraph->imageViews.clear();


        delete(renderGraph);
    }



}  // namespace g2::gfx