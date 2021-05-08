//
// Created by felix on 24/04/2021.
//

#include "image.h"
#include "Buffer.h"
#include <cstring>
#include <vector>
#include <zstd.h>
#include <g2/gfx/image_generated.h>

static VkImageType getImageTypeFromDimensions(uint32_t dim) {
    switch (dim) {
        case 1:
            return VK_IMAGE_TYPE_1D;
        default:
        case 2:
            return VK_IMAGE_TYPE_2D;
        case 3:
            return VK_IMAGE_TYPE_3D;
    }
};


g2::gfx::Image g2::gfx::loadImage(VkDevice device, UploadQueue* uploadQueue, VmaAllocator allocator, std::span<const char> data) {


    const ImageDef* imageDef = GetImageDef(data.data());


    //Create image

    Image image{};

    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = getImageTypeFromDimensions(2),
        .format = static_cast<VkFormat>(imageDef->format()),
        .extent = VkExtent3D {
            .width = imageDef->width(),
            .height = imageDef->height(),
            .depth = imageDef->depth(),
        },
        .mipLevels = imageDef->levels(),
        .arrayLayers = 1 * (/*cubemap*/false ? 6 : 1),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    vmaCreateImage(allocator, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr);

    std::vector<VkBufferImageCopy> copyRegions;
    copyRegions.reserve(imageDef->levels());

    {
        uint64_t offset = 0;
        uint64_t width = imageDef->width();
        uint64_t height = imageDef->height();
        uint64_t depth = imageDef->depth();
        for (int level = 0; level < imageDef->levels(); level++) {

            auto &region = copyRegions.emplace_back();
            region.bufferOffset = offset;
            offset += width * height; //TODO take format into account
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = level;
            region.imageSubresource.baseArrayLayer = 0; //TODO layers and cubemaps
            region.imageSubresource.layerCount = 1; // TODO layers and cubemaps
            region.imageOffset.x = 0;
            region.imageOffset.y = 0;
            region.imageOffset.z = 0;
            region.imageExtent.width = width;
            region.imageExtent.height = height;
            region.imageExtent.depth = depth;

            width /= 2;
            height /= 2;
            depth /= 2;
        }
    }


    auto dataSize = ZSTD_getDecompressedSize(imageDef->data()->data(), imageDef->data()->size());
    void* scratchPtr = uploadQueue->queueImageUpload(dataSize, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, copyRegions);
    ZSTD_decompress(scratchPtr, dataSize, imageDef->data()->data(), imageDef->data()->size());

    VkImageSubresourceRange imageRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = imageDef->levels(),
            .baseArrayLayer = 0,
            .layerCount = 1,
    };

    // Create the view
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = static_cast<VkFormat>(imageDef->format()),
        .components = VkComponentMapping {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = imageRange,
    };

    vkCreateImageView(device, &viewInfo, nullptr, &image.view);

    return image;

}

g2::AssetAddResult g2::gfx::ImageAssetManager::add_asset(std::span<const char> data) {

    auto& image = images.emplace_back(loadImage(device, uploadQueue, allocator, data));

    uint32_t index = images.size() - 1;

    VkDescriptorImageInfo imageInfo {
            .sampler = sampler,
            .imageView = image.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet writeDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = resourceDescriptorSet,
            .dstBinding = 1,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

    return AssetAddResult {
        .index = static_cast<uint32_t>(index),
        .patches = {},
    };
}

const char *g2::gfx::ImageAssetManager::ext() {
    return ".g2img";
}
