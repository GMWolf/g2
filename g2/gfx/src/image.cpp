//
// Created by felix on 24/04/2021.
//

#include "image.h"
#include <ktx.h>
#include <ktxvulkan.h>
#include "Buffer.h"
#include <cstring>
#include <vector>

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


struct CbData {
    std::vector<VkBufferImageCopy> regions;
    VkDeviceSize offset;
    uint32_t numFaces;
    uint32_t numLayers;
    uint8_t* dest;
    uint32_t elementSize;
    uint32_t numDimensions;

};

ktx_error_code_e tilingCallback(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userData) {
    auto* ud = static_cast<CbData*>(userData);

    auto& region = ud->regions.emplace_back();
    region.bufferOffset = ud->offset;
    ud->offset += faceLodSize;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = miplevel;
    region.imageSubresource.baseArrayLayer = face;
    region.imageSubresource.layerCount = ud->numLayers * ud->numFaces;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = depth;

    return KTX_SUCCESS;
};


g2::gfx::Image g2::gfx::loadImage(VkDevice device, VkCommandBuffer cmd, VmaAllocator allocator, std::span<char> data) {


    ktxTexture2* ktxTex;
    ktxTexture2_CreateFromMemory(reinterpret_cast<uint8_t*>(data.data()), data.size(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktxTex);
    ktxTexture2_TranscodeBasis(ktxTex, KTX_TTF_BC7_RGBA, 0);

    // Create scratch buffer

    Buffer scratchBuffer{};
    VkBufferCreateInfo scratchBufferInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = ktxTex->dataSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VmaAllocationCreateInfo scratchAllocationInfo {
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
    };
    createBuffer(allocator, &scratchBufferInfo, &scratchAllocationInfo, &scratchBuffer);
    void* scratchPtr;
    vmaMapMemory(allocator, scratchBuffer.allocation, &scratchPtr);

    if(ktxTex->pData) {
        memcpy(scratchPtr, ktxTex->pData, ktxTex->dataSize);
    } else {
        ktxTexture_LoadImageData(ktxTexture(ktxTex), static_cast<ktx_uint8_t *>(scratchPtr), scratchBufferInfo.size);
    }

    vmaUnmapMemory(allocator, scratchBuffer.allocation);

    //Create image

    Image image;


    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = getImageTypeFromDimensions(ktxTex->numDimensions),
        .format = static_cast<VkFormat>(ktxTex->vkFormat),
        .extent = VkExtent3D {
            .width = ktxTex->baseWidth,
            .height = ktxTex->baseHeight,
            .depth = ktxTex->baseDepth,
        },
        .mipLevels = ktxTex->numLevels,
        .arrayLayers = ktxTex->numLayers * (ktxTex->isCubemap ? 6 : 1),
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

    CbData cbData{};
    cbData.regions.clear();
    cbData.offset = 0;
    cbData.numFaces = ktxTex->numFaces;
    cbData.numLayers = ktxTex->numLayers;
    cbData.dest = static_cast<uint8_t *>(scratchPtr),
    cbData.elementSize = ktxTexture_GetElementSize(ktxTexture(ktxTex));
    cbData.numDimensions = ktxTex->numDimensions;

    ktxTexture_IterateLevels(ktxTexture(ktxTex), tilingCallback, &cbData);

    VkImageSubresourceRange imageRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = ktxTex->numLevels,
            .baseArrayLayer = 0,
            .layerCount = ktxTex->numLayers * (ktxTex->isCubemap ? 6 : 1),
    };

    {
        VkImageMemoryBarrier imageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image.image,
                .subresourceRange = imageRange,
        };

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &imageMemoryBarrier);
    }

    vkCmdCopyBufferToImage(cmd, scratchBuffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cbData.regions.size(), cbData.regions.data());

    {
        VkImageMemoryBarrier imageMemoryBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image.image,
                .subresourceRange = imageRange,
        };

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &imageMemoryBarrier);
    }


    //TODO Leak the buffer for now untill we have a real upload queue
    //vmaDestroyBuffer(allocator, scratchBuffer.buffer, scratchBuffer.allocation);
    ktxTexture_Destroy(ktxTexture(ktxTex));


    // Create the view
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = static_cast<VkFormat>(ktxTex->vkFormat),
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
