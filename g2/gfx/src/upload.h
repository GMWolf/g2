//
// Created by felix on 25/04/2021.
//

#ifndef G2_UPLOAD_H
#define G2_UPLOAD_H

#include "Buffer.h"
#include <vector>
#include <vk_mem_alloc.h>
#include <span>
#include <variant>
#include <memory>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <mutex>

namespace g2::gfx {

    struct UploadSource {
        std::span<char> data;
        bool compressed; // Whether the data is zstd compressed or not.

        size_t getUncompressedDataSize() const;
    };

    struct BufferUploadTarget {
        VkBuffer targetBuffer;
        size_t offset;
    };

    struct ImageUploadTarget {
        VkImage targetImage;
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        std::vector<VkBufferImageCopy> regions;
    };

    struct UploadJob {
        uint32_t priority;
        UploadSource source;
        std::variant<BufferUploadTarget, ImageUploadTarget> target;
    };

    struct UploadJobPriorityComparator {
        inline bool operator()(const UploadJob& lhs, const UploadJob& rhs) {
            return lhs.priority < rhs.priority;
        }
    };

    struct UploadQueue {
        std::priority_queue<UploadJob, std::vector<UploadJob>, UploadJobPriorityComparator> jobs;

        // Upload queue works in upload frames. They are separate frames to graphics frames
        static const uint32_t uploadFrameCount = 4;
        static const size_t stagingBufferSize = 10 * 1024 * 1024;

        VkCommandPool commandPool;

        struct Frame {
            VkFence fence;
            VkCommandBuffer commandBuffer;
            LinearBuffer stagingBuffer;
            void* map;
        };

        Frame frames[uploadFrameCount];

        std::queue<uint64_t> pendingFrames; // Frames that need submitting
        std::queue<uint64_t> freeFrames; // Frames ready to be written to
        bool frameInFlight[uploadFrameCount]; // wether the frame is in flight

        std::mutex jobQueueMutex;
        std::condition_variable jobQueueCV;
        std::mutex freeQueueMutex;
        std::condition_variable freeQueueCV;

        std::mutex pendingQueueMutex;


        int currentFrame = -1;

        //Called from client side
        void update(VkDevice device, VkQueue queue);

        [[noreturn]] void processJobs();

        bool recordBufferUpload(size_t frameIndex, const UploadSource& source, BufferUploadTarget& target);
        bool recordImageUpload(size_t frameIndex, const UploadSource& source, ImageUploadTarget& target);
        bool recordJob(size_t frameIndex, UploadJob& job);

        void addJob(UploadJob&& job);

    };

    void createUploadQueue(VkDevice device, VmaAllocator allocator, uint32_t queueFamily, UploadQueue* uploadQueue);
    void destroyUploadQueue(VkDevice deive, VmaAllocator allocator, UploadQueue* uploadQueue);

}

#endif //G2_UPLOAD_H
