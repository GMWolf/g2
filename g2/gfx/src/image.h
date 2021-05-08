//
// Created by felix on 24/04/2021.
//

#ifndef G2_IMAGE_H
#define G2_IMAGE_H

#include <vulkan/vulkan.h>
#include <span>
#include <vk_mem_alloc.h>
#include "upload.h"
#include <g2/assets/asset_registry.h>

namespace g2::gfx {

    struct Image {
        VkImage image;
        VmaAllocation allocation;
        VkImageView view;
    };


    struct ImageAssetManager : public IAssetManager {

        struct UploadJob {
            VkImage image;
            std::vector<VkBufferImageCopy> copyRegions;
            std::span<const uint8_t> data;
        };

        std::vector<UploadJob> jobs;

        // TODO remove these members and use a queue queried from gfx instance.
        VkDevice device;
        UploadQueue* uploadQueue;
        VmaAllocator allocator;
        VkDescriptorSet resourceDescriptorSet;
        VkSampler sampler;

        std::vector<Image> images;

        void runJobs();

        AssetAddResult add_asset(std::span<const char> data) override;
        const char *ext() override;
    };


}


#endif //G2_IMAGE_H
