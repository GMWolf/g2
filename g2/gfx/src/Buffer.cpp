//
// Created by felix on 23/04/2021.
//

#include "Buffer.h"
void g2::gfx::createBuffer(VmaAllocator allocator,
                           const VkBufferCreateInfo *bufferCreateInfo,
                           const VmaAllocationCreateInfo *allocationCreateInfo,
                           g2::gfx::Buffer *buffer) {

  VmaAllocationInfo allocInfo;

  vmaCreateBuffer(allocator, bufferCreateInfo, allocationCreateInfo, &buffer->buffer, &buffer->allocation, &allocInfo);

  buffer->size = allocInfo.size;

}

void g2::gfx::createLinearBuffer(VmaAllocator allocator,
                                 const VkBufferCreateInfo *bufferCreateInfo,
                                 const VmaAllocationCreateInfo *allocationInfo,
                                 g2::gfx::LinearBuffer *buffer) {
  createBuffer(allocator, bufferCreateInfo, allocationInfo, buffer);
  buffer->head = 0;
}

void g2::gfx::uploadBuffer(VkCommandBuffer cmd, VkBuffer src,
                           size_t srcOffset, size_t size, VkBuffer dst,
                           size_t dstOffset) {

  VkBufferCopy region = {
      .srcOffset = srcOffset,
      .dstOffset = dstOffset,
      .size = size
  };

  vkCmdCopyBuffer(cmd, src, dst, 1, &region);

  VkBufferMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = dst,
      .offset = dstOffset,
      .size = size,
  };

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);

}
g2::gfx::BufferRegion g2::gfx::allocateFromLinearBuffer(
    g2::gfx::LinearBuffer *linearBuffer, size_t size, size_t align) {

  const uintptr_t aligned = (linearBuffer->head - 1u + align) & -align;

  if(aligned + size > linearBuffer->size) {
    return {0, 0};
  }

  linearBuffer->head = aligned + size;

  return {aligned, size};


}

