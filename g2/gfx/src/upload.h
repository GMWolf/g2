//
// Created by felix on 25/04/2021.
//

#ifndef G2_UPLOAD_H
#define G2_UPLOAD_H

#include "Buffer.h"
#include <vector>
#include <vk_mem_alloc.h>
#include <span>

namespace g2::gfx {

    struct UploadQueue {
        // Upload queue works in upload frames. They are separate frames to graphics frames
        static const uint32_t uploadFrameCount = 4;
        static const size_t stagingBufferSize = 20 * 1024 * 1024;
        uint32_t currentUploadFrame;

        VkFence fences[uploadFrameCount];
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffers[uploadFrameCount];
        LinearBuffer stagingBuffer[uploadFrameCount];
        void* stagingBufferMap[uploadFrameCount];

        uint32_t workCount;

        void* queueBufferUpload(size_t numBytes, VkBuffer buffer, size_t offset);
        void* queueImageUpload(size_t numBytes,  VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, std::span<VkBufferImageCopy> regions);
        void submit(VkDevice device, VkQueue queue);
    };

    void createUploadQueue(VkDevice device, VmaAllocator allocator, uint32_t queueFamily, UploadQueue* uploadQueue);

}

#endif //G2_UPLOAD_H
