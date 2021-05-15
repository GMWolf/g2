//
// Created by felix on 25/04/2021.
//

#include "upload.h"
#include <zstd.h>
#include <cstring>
#include <iostream>
#include <cassert>

namespace g2::gfx {

    bool UploadQueue::recordBufferUpload(size_t frameIndex, const UploadSource& source, BufferUploadTarget& target) {

        size_t numBytes = source.getUncompressedDataSize();
        assert(numBytes <= stagingBufferSize);


        auto stagingAlloc = allocateFromLinearBuffer(&frames[frameIndex].stagingBuffer, numBytes, 1);

        if (!stagingAlloc) {
            return false;
        }

        if (source.compressed) {
            ZSTD_decompress((char *) frames[frameIndex].map + stagingAlloc.offset, stagingAlloc.size,
                            source.data.data(), source.data.size_bytes());
        } else {
            memcpy((char *) frames[frameIndex].map + stagingAlloc.offset, source.data.data(),
                   source.data.size_bytes());
        }

        VkBufferCopy bufferCopy{
                .srcOffset = stagingAlloc.offset,
                .dstOffset = target.offset,
                .size = numBytes,
        };

        vkCmdCopyBuffer(frames[frameIndex].commandBuffer, frames[frameIndex].stagingBuffer.buffer, target.targetBuffer, 1,
                        &bufferCopy);

        return true;
    }

    bool UploadQueue::recordImageUpload(size_t frameIndex, const UploadSource& source, ImageUploadTarget& target) {

        assert(frameIndex != -1);

        size_t numBytes = source.getUncompressedDataSize();
        assert(numBytes <= stagingBufferSize);

        auto stagingAlloc = allocateFromLinearBuffer(&frames[frameIndex].stagingBuffer, numBytes, 16);
        if (!stagingAlloc) {
            return false;
        }

        if (source.compressed) {
            ZSTD_decompress((char *) frames[frameIndex].map + stagingAlloc.offset, stagingAlloc.size,
                            source.data.data(), source.data.size_bytes());
        } else {
            memcpy((char *) frames[frameIndex].map + stagingAlloc.offset, source.data.data(),
                   source.data.size_bytes());
        }



        // Patch region buffer offset, get range
        uint32_t minMip = UINT32_MAX;
        uint32_t maxMip = 0;
        uint32_t minLayer = UINT32_MAX;
        uint32_t maxLayer = 0;

        //Patch offsets and get minmax
        for (auto &region : target.regions) {
            region.bufferOffset += stagingAlloc.offset;

            minMip = std::min(region.imageSubresource.mipLevel, minMip);
            maxMip = std::max(region.imageSubresource.mipLevel, maxMip);
            minLayer = std::min(region.imageSubresource.baseArrayLayer, minLayer);
            maxLayer = std::max(region.imageSubresource.baseArrayLayer + region.imageSubresource.layerCount - 1,
                                maxLayer);
        }

        VkImageSubresourceRange imageRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = minMip,
                .levelCount = 1 + maxMip - minMip,
                .baseArrayLayer = minLayer,
                .layerCount = 1 + maxLayer - minLayer,
        };
        {

            VkImageMemoryBarrier imageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = target.oldLayout,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = target.targetImage,
                    .subresourceRange = imageRange,
            };

            vkCmdPipelineBarrier(frames[frameIndex].commandBuffer,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &imageMemoryBarrier);
        }

        vkCmdCopyBufferToImage(frames[frameIndex].commandBuffer, frames[frameIndex].stagingBuffer.buffer,
                               target.targetImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, target.regions.size(),
                               target.regions.data());

        {
            VkImageMemoryBarrier imageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = target.newLayout,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = target.targetImage,
                    .subresourceRange = imageRange,
            };

            vkCmdPipelineBarrier(frames[frameIndex].commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &imageMemoryBarrier);
        }

        return true;
    }


    bool UploadQueue::recordJob(size_t frameIndex, UploadJob &job) {
        if (std::holds_alternative<BufferUploadTarget>(job.target)) {
            return recordBufferUpload(frameIndex, job.source, std::get<BufferUploadTarget>(job.target));
        } else if (std::holds_alternative<ImageUploadTarget>(job.target)) {
            return recordImageUpload(frameIndex, job.source, std::get<ImageUploadTarget>(job.target));
        }
        return false;
    }

    void UploadQueue::update(VkDevice device, VkQueue queue) {

        std::unique_lock<std::mutex> flk(freeQueueMutex, std::defer_lock);
        std::unique_lock<std::mutex> plk(pendingQueueMutex, std::defer_lock);

        //submit pending (pending => in flight)
        //TODO submit more than one

        plk.lock();
        if (!pendingFrames.empty()) {
            VkSubmitInfo submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &frames[pendingFrames.front()].commandBuffer,
            };

            vkResetFences(device, 1, &frames[pendingFrames.front()].fence);
            vkQueueSubmit(queue, 1, &submitInfo, frames[pendingFrames.front()].fence);
            frameInFlight[pendingFrames.front()] = true;
            pendingFrames.pop();
        }
        plk.unlock();

        //Check inflight frames (in flight => free)
        for (uint64_t frameIndex = 0; frameIndex < uploadFrameCount; frameIndex++) {
            if (frameInFlight[frameIndex]) {
                if (vkGetFenceStatus(device, frames[frameIndex].fence) == VK_SUCCESS) {
                    flk.lock();
                    freeFrames.push(frameIndex);
                    flk.unlock();
                    freeQueueCV.notify_one();
                    frameInFlight[frameIndex] = false;
                }
            }
        }
    }

    [[noreturn]] void UploadQueue::processJobs() {

        std::unique_lock<std::mutex> jlk(jobQueueMutex, std::defer_lock);
        std::unique_lock<std::mutex> flk(freeQueueMutex, std::defer_lock);
        std::unique_lock<std::mutex> plk(pendingQueueMutex, std::defer_lock);

        while (true) {


            //Flush if jobs is empty
            jlk.lock();
            if (currentFrame != -1) {
                if (jobs.empty()) {
                    jlk.unlock();
                    vkEndCommandBuffer(frames[currentFrame].commandBuffer);
                    plk.lock();
                    pendingFrames.push(currentFrame);
                    plk.unlock();
                    currentFrame = -1;
                    jlk.lock();
                }
            }

            // acquire job
            jobQueueCV.wait(jlk, [this] {
                return !jobs.empty();
            });

            UploadJob job = std::move(jobs.top());
            jobs.pop();
            std::cout << "job count " << jobs.size() << std::endl;
            jlk.unlock();


            //Process job
            bool executedJob = false;
            while (executedJob == false) {
                //Acquire a frame
                if (currentFrame == -1) {
                    flk.lock();
                    freeQueueCV.wait(flk, [this] {
                        return !freeFrames.empty();
                    });
                    currentFrame = freeFrames.front();
                    freeFrames.pop();
                    flk.unlock();
                    vkResetCommandBuffer(frames[currentFrame].commandBuffer, 0);
                    frames[currentFrame].stagingBuffer.head = 0;
                    VkCommandBufferBeginInfo beginInfo{
                            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                            .pInheritanceInfo = nullptr,
                    };
                    vkBeginCommandBuffer(frames[currentFrame].commandBuffer, &beginInfo);
                }

                //execute job
                if (recordJob(currentFrame, job)) {
                    executedJob = true;
                } else { // Not enough space in current frame. Submit and continue trying
                    vkEndCommandBuffer(frames[currentFrame].commandBuffer);
                    plk.lock();
                    pendingFrames.push(currentFrame);
                    plk.unlock();
                    currentFrame = -1;
                }
            }
        }
    }

    void UploadQueue::addJob(UploadJob &&job) {
        std::unique_lock<std::mutex> jlk(jobQueueMutex);
        jobs.push(std::move(job));
        jlk.unlock();
        jobQueueCV.notify_one();
    }

    void createUploadQueue(VkDevice device, VmaAllocator allocator, uint32_t queueFamily, UploadQueue *uploadQueue) {

        VkBufferCreateInfo scratchBufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = UploadQueue::stagingBufferSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo scratchAllocationInfo{
                .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

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
                .commandBufferCount = 1,
        };

        VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT};

        for (int i = 0; i < UploadQueue::uploadFrameCount; i++) {
            createLinearBuffer(allocator, &scratchBufferInfo, &scratchAllocationInfo,
                               &uploadQueue->frames[i].stagingBuffer);
            vmaMapMemory(allocator, uploadQueue->frames[i].stagingBuffer.allocation, &uploadQueue->frames[i].map);
            vkAllocateCommandBuffers(device, &commandBufferInfo, &uploadQueue->frames[i].commandBuffer);
            vkCreateFence(device, &fenceInfo, nullptr, &uploadQueue->frames[i].fence);
            uploadQueue->freeFrames.push(i);
        }

    }

    size_t UploadSource::getUncompressedDataSize() const {
        if (compressed) {
            return ZSTD_getDecompressedSize(data.data(), data.size_bytes());
        } else {
            return data.size_bytes();
        }
    }
}