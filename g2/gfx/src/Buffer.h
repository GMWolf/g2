//
// Created by felix on 23/04/2021.
//

#ifndef G2_G2_GFX_SRC_BUFFER_H_
#define G2_G2_GFX_SRC_BUFFER_H_

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace g2::gfx {

struct Buffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  size_t size;
};

struct BufferRegion {
  size_t offset;
  size_t size;

  inline operator bool() const {
      return size > 0;
  }
};

struct LinearBuffer : public Buffer{
  uintptr_t head;
};


void createBuffer(VmaAllocator allocator, const VkBufferCreateInfo* bufferCreateInfo, const VmaAllocationCreateInfo* allocationInfo, Buffer* buffer);
void createLinearBuffer(VmaAllocator allocator, const VkBufferCreateInfo* bufferCreateInfo, const VmaAllocationCreateInfo* allocationInfo, LinearBuffer* buffer);

void uploadBuffer(VkCommandBuffer cmd, VkBuffer src,
                  size_t srcOffset, size_t size, VkBuffer dst,
                  size_t dstOffset);

BufferRegion allocateFromLinearBuffer(LinearBuffer* linearBuffer, size_t size, size_t align);

}
#endif  // G2_G2_GFX_SRC_BUFFER_H_
