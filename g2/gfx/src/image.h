//
// Created by felix on 24/04/2021.
//

#ifndef G2_IMAGE_H
#define G2_IMAGE_H

#include <vulkan/vulkan.h>
#include <span>
#include <vk_mem_alloc.h>

namespace g2::gfx {

    struct Image {
        VkImage image;
        VmaAllocation allocation;
        VkImageView view;
    };

    Image loadImage(VkDevice device, VkCommandBuffer cmd, VmaAllocator allocator, std::span<char> data);

}


#endif //G2_IMAGE_H
