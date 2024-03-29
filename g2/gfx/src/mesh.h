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
#include <g2/assets/asset_registry.h>
#include <vector>
#include <g2/hat.h>
#include <glm/vec3.hpp>

namespace g2::gfx {

struct Meshlet {
    glm::vec3 center;
    float radius;
    glm::vec3 coneApex;
    glm::vec3 coneAxis;
    float coneCutoff;
    uint32_t triangleOffset;
    uint32_t triangleCount;
    uint32_t vertexOffset;
};

struct Primitive {
  size_t positionOffset;
  size_t normalOffset;
  size_t texcoordOffset;
  size_t tangentOffset;
  size_t bitangentOffset;
  size_t baseIndex;
  uint32_t material;

  std::vector<Meshlet> meshlets;
};

struct Mesh {
    std::vector<Primitive> primitives;
};


struct MeshBuffer {
  LinearBuffer indexBuffer;
  LinearBuffer vertexBuffer;
};

void initMeshBuffer(VmaAllocator allocator, MeshBuffer* meshBuffer);
void destroyMeshBuffer(VmaAllocator allocator, MeshBuffer* meshBuffer);


Mesh addMesh(UploadQueue* uploadQueue, MeshBuffer* meshBuffer, const MeshData* meshData);

struct MeshAssetManager : public IAssetManager{

    // TODO use some sort of queue
    UploadQueue* uploadQueue;
    MeshBuffer* meshBuffer;

    g2::hat<Mesh> meshes;

    uint32_t nextMeshIndex = 0;

    AssetAddResult add_asset(std::span<const char> data) override;
    const char *ext() override;
};

}


#endif  // G2_G2_GFX_SRC_MESH_H_
