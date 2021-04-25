//
// Created by felix on 23/04/2021.
//

#include "mesh.h"
#include <cstring>
#include <cassert>

void g2::gfx::initMeshBuffer(VmaAllocator allocator,
                             g2::gfx::MeshBuffer *meshBuffer) {

  size_t vertexBufferSize = 256 * 1024 * 1024;
  size_t indexBufferSize = 16 * 1024 * 1024;

  VkBufferCreateInfo vertexBufferInfo {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = vertexBufferSize,
    .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VmaAllocationCreateInfo vertexAllocationInfo {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  createLinearBuffer(allocator, &vertexBufferInfo, &vertexAllocationInfo, &meshBuffer->vertexBuffer);


  VkBufferCreateInfo indexBufferInfo {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = indexBufferSize,
      .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VmaAllocationCreateInfo indexAllocationInfo {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  createLinearBuffer(allocator, &indexBufferInfo, &indexAllocationInfo, &meshBuffer->indexBuffer);


  VkBufferCreateInfo scratchBufferInfo {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = indexBufferSize,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VmaAllocationCreateInfo scratchAllocationInfo {
      .usage = VMA_MEMORY_USAGE_CPU_ONLY,
  };

  createBuffer(allocator, &scratchBufferInfo, &scratchAllocationInfo, &meshBuffer->scratchBuffer);
  vmaMapMemory(allocator, meshBuffer->scratchBuffer.allocation, &meshBuffer->scratchPtr);
  assert(meshBuffer->scratchPtr);
}


g2::gfx::Mesh g2::gfx::addMesh(VkCommandBuffer cmd, g2::gfx::MeshBuffer *meshBuffer,
                      g2::gfx::MeshFormat *meshFormat, void *vertexData,
                      size_t vertexCount, void *indexData, size_t indexCount) {

  assert(meshFormat->indexType = VK_INDEX_TYPE_UINT32);
  const size_t indexByteSize = 4;

  size_t vertexBytes = vertexCount * meshFormat->vertexByteSize;
  size_t indexBytes = indexByteSize * indexCount;

  memcpy(meshBuffer->scratchPtr, vertexData, vertexBytes);
  auto vertexAlloc = allocateFromLinearBuffer(&meshBuffer->vertexBuffer, vertexBytes, meshFormat->vertexByteSize);
  assert(vertexAlloc.size);
  uploadBuffer(cmd, meshBuffer->scratchBuffer.buffer, 0, vertexBytes, meshBuffer->vertexBuffer.buffer, vertexAlloc.offset );

  memcpy((char*)meshBuffer->scratchPtr + vertexBytes, indexData, indexBytes);
  auto indexAlloc = allocateFromLinearBuffer(&meshBuffer->indexBuffer, indexBytes, indexByteSize);
  assert(indexAlloc.size);
  uploadBuffer(cmd, meshBuffer->scratchBuffer.buffer, vertexBytes, indexBytes, meshBuffer->indexBuffer.buffer, indexAlloc.offset);

  Primimitive prim {
          .meshFormat = *meshFormat,
          .baseVertex = vertexAlloc.offset / meshFormat->vertexByteSize,
          .vertexCount = vertexCount,
          .baseIndex = indexAlloc.offset / indexByteSize,
          .indexCount = indexCount,
  };

  return Mesh {
          {prim}
  };

}

g2::gfx::Mesh g2::gfx::addMesh(VkCommandBuffer cmd, MeshBuffer* meshBuffer, const g2::gfx::MeshData *meshData) {

    Mesh mesh;

    for(auto primitive : *meshData->primitives()) {
        MeshFormat format {
                .vertexByteSize = primitive->vertexAlignment(),
                .indexType = VK_INDEX_TYPE_UINT32,
        };

        //Todo actually append
        return addMesh(cmd, meshBuffer, &format, (void*)primitive->vertexData()->data(), primitive->vertexCount(), (void*)primitive->indices()->data(), primitive->indices()->size());
    }




    return mesh;


}
