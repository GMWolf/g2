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
}


g2::gfx::Mesh g2::gfx::addMesh(UploadQueue* uploadQueue, g2::gfx::MeshBuffer *meshBuffer,
                      g2::gfx::MeshFormat *meshFormat, void *vertexData,
                      size_t vertexCount, void *indexData, size_t indexCount) {

  assert(meshFormat->indexType = VK_INDEX_TYPE_UINT32);
  const size_t indexByteSize = 4;

  size_t vertexBytes = vertexCount * meshFormat->vertexByteSize;
  size_t indexBytes = indexByteSize * indexCount;

  auto vertexAlloc = allocateFromLinearBuffer(&meshBuffer->vertexBuffer, vertexBytes, meshFormat->vertexByteSize);
  assert(vertexAlloc);
  void* vertexScratch = uploadQueue->queueBufferUpload(vertexBytes, meshBuffer->vertexBuffer.buffer, vertexAlloc.offset);
  memcpy(vertexScratch, vertexData, vertexBytes);


  auto indexAlloc = allocateFromLinearBuffer(&meshBuffer->indexBuffer, indexBytes, indexByteSize);
  assert(indexAlloc.size);
  void* indexScratch = uploadQueue->queueBufferUpload(indexBytes, meshBuffer->indexBuffer.buffer, indexAlloc.offset);
  memcpy(indexScratch, indexData, indexBytes);

  Primitive prim {
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

g2::gfx::Mesh g2::gfx::addMesh(UploadQueue* uploadQueue, MeshBuffer* meshBuffer, const g2::gfx::MeshData *meshData) {

    Mesh mesh;

    for(auto primitive : *meshData->primitives()) {
        MeshFormat format {
                .vertexByteSize = primitive->vertexAlignment(),
                .indexType = VK_INDEX_TYPE_UINT32,
        };

        //Todo actually append primitives
        return addMesh(uploadQueue, meshBuffer, &format, (void*)primitive->vertexData()->data(), primitive->vertexCount(), (void*)primitive->indices()->data(), primitive->indices()->size());
    }


    return mesh;
}

g2::AssetAddResult g2::gfx::MeshAssetManager::add_asset(std::span<char> data) {
    auto meshData = g2::gfx::GetMeshData(data.data());

    auto& mesh = meshes.emplace_back(addMesh(uploadQueue, meshBuffer, meshData));

    uint32_t index = meshes.size() - 1;

    return AssetAddResult {
        .index = index,
        .patches = {},
    };

}

const char *g2::gfx::MeshAssetManager::ext() {
    return ".g2mesh";
}
