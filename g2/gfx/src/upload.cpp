//
// Created by felix on 25/04/2021.
//

#include "upload.h"

namespace g2::gfx {

    void * UploadQueue::queueBufferUpload(size_t numBytes, VkBuffer buffer, size_t offset) {

        auto stagingAlloc = allocateFromLinearBuffer(&stagingBuffer[currentUploadFrame], numBytes, 1);
        if (!stagingAlloc) {
            return nullptr;
        }

        VkBufferCopy bufferCopy{
                .srcOffset = stagingAlloc.offset,
                .dstOffset = offset,
                .size = numBytes,
        };

        vkCmdCopyBuffer(commandBuffers[currentUploadFrame], stagingBuffer[currentUploadFrame].buffer, buffer, 1,
                        &bufferCopy);

        workCount ++;

        return (char *) stagingBufferMap[currentUploadFrame] + stagingAlloc.offset;
    }

    void  UploadQueue::submit(VkDevice device, VkQueue queue) {

        vkEndCommandBuffer(commandBuffers[currentUploadFrame]);

        if (workCount > 0) {
            // Submit current work
            VkSubmitInfo submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffers[currentUploadFrame],

            };

            vkResetFences(device, 1, &fences[currentUploadFrame]);
            vkQueueSubmit(queue, 1, &submitInfo, fences[currentUploadFrame]);


            workCount = 0;
            // Acquire new upload frame
            if (++currentUploadFrame > uploadFrameCount) {
                currentUploadFrame = 0;
            }

            vkWaitForFences(device, 1, &fences[currentUploadFrame], VK_TRUE, UINT64_MAX);
            vkResetCommandBuffer(commandBuffers[currentUploadFrame], 0);
            VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                    .pInheritanceInfo = nullptr,
            };
            vkBeginCommandBuffer(commandBuffers[currentUploadFrame], &beginInfo);
            stagingBuffer[currentUploadFrame].head = 0;
        }


    }


    void  createUploadQueue(VkDevice device, VmaAllocator allocator, uint32_t queueFamily, UploadQueue* uploadQueue) {

        VkBufferCreateInfo scratchBufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = UploadQueue::stagingBufferSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo scratchAllocationInfo{
                .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

        for (int i = 0; i < UploadQueue::uploadFrameCount; i++) {
            createLinearBuffer(allocator, &scratchBufferInfo, &scratchAllocationInfo, &uploadQueue->stagingBuffer[i]);
            vmaMapMemory(allocator, uploadQueue->stagingBuffer[i].allocation, &uploadQueue->stagingBufferMap[i]);
        }

        VkCommandPoolCreateInfo commandPoolInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = queueFamily,
        };
        vkCreateCommandPool(device, &commandPoolInfo, nullptr, &uploadQueue->commandPool);

        VkCommandBufferAllocateInfo commandBufferInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = uploadQueue->commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = g2::gfx::UploadQueue::uploadFrameCount,
        };

        vkAllocateCommandBuffers(device, &commandBufferInfo, uploadQueue->commandBuffers);

        VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for(int i = 0; i < UploadQueue::uploadFrameCount; i++) {
            vkCreateFence(device, &fenceInfo, nullptr, &uploadQueue->fences[i]);
        }


        uploadQueue->currentUploadFrame = 0;
        uploadQueue->workCount = 0;

        VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(uploadQueue->commandBuffers[0], &beginInfo);
    }

}