//
// Created by felix on 23/04/2021.
//

#ifndef G2_G2_GFX_SRC_MESH_H_
#define G2_G2_GFX_SRC_MESH_H_

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Buffer.h"
#include <vector>
#include <g2/gfx/mesh_generated.h>
#include "upload.h"

namespace g2::gfx {

struct MeshFormat {
  size_t vertexByteSize;  // Size in bytes of vertex data, including padding
  VkIndexType indexType;
};

struct Primitive {
  MeshFormat meshFormat;
  size_t baseVertex;  // Offset into mesh buffer in vertices
  size_t vertexCount;
  size_t baseIndex;
  size_t indexCount;
};

struct Mesh {
    std::vector<Primitive> primitives;
};


struct MeshBuffer {
  LinearBuffer indexBuffer;
  LinearBuffer vertexBuffer;
};

void initMeshBuffer(VmaAllocator allocator, MeshBuffer* meshBuffer);

Mesh addMesh(UploadQueue* uploadQueue, MeshBuffer* meshBuffer, MeshFormat* meshFormat, void* vertexData, size_t vertexCount, void* indexData, size_t indexCount);

Mesh addMesh(UploadQueue* uploadQueue, MeshBuffer* meshBuffer, const MeshData* meshData);

}


#endif  // G2_G2_GFX_SRC_MESH_H_
