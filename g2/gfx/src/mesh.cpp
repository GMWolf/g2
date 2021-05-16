//
// Created by felix on 23/04/2021.
//

#include "mesh.h"
#include <cstring>
#include <cassert>

void g2::gfx::initMeshBuffer(VmaAllocator allocator,
                             g2::gfx::MeshBuffer *meshBuffer) {

    size_t vertexBufferSize = 512 * 1024 * 1024;
    size_t indexBufferSize = 64 * 1024 * 1024;

    VkBufferCreateInfo vertexBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = vertexBufferSize,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo vertexAllocationInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    createLinearBuffer(allocator, &vertexBufferInfo, &vertexAllocationInfo, &meshBuffer->vertexBuffer);


    VkBufferCreateInfo indexBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = indexBufferSize,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo indexAllocationInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    createLinearBuffer(allocator, &indexBufferInfo, &indexAllocationInfo, &meshBuffer->indexBuffer);
}


g2::gfx::Primitive g2::gfx::addMeshPrimitive(UploadQueue *uploadQueue, g2::gfx::MeshBuffer *meshBuffer,
                               g2::gfx::MeshFormat *meshFormat, void *vertexData,
                               size_t vertexCount, void *indexData, size_t indexCount) {

    assert(meshFormat->indexType = VK_INDEX_TYPE_UINT32);
    const size_t indexByteSize = 4;

    size_t vertexBytes = vertexCount * meshFormat->vertexByteSize;
    size_t indexBytes = indexByteSize * indexCount;

    auto vertexAlloc = allocateFromLinearBuffer(&meshBuffer->vertexBuffer, vertexBytes, meshFormat->vertexByteSize);
    assert(vertexAlloc.size);

    uploadQueue->jobs.push({
        .priority = UINT16_MAX,
        .source = UploadSource{
            .data = std::span((char *) vertexData, vertexBytes),
            .compressed = false,
        },
        .target = BufferUploadTarget {
            .targetBuffer = meshBuffer->vertexBuffer.buffer,
            .offset = vertexAlloc.offset
        },
    });


    auto indexAlloc = allocateFromLinearBuffer(&meshBuffer->indexBuffer, indexBytes, indexByteSize);
    assert(indexAlloc.size);
    uploadQueue->addJob(UploadJob{
            .priority = UINT16_MAX,
            .source = UploadSource{
                    .data = std::span((char *) indexData, indexBytes),
                    .compressed = false,
            },
            .target = BufferUploadTarget {
                .targetBuffer = meshBuffer->indexBuffer.buffer,
                .offset = indexAlloc.offset
            },
    });

    assert((vertexAlloc.offset / meshFormat->vertexByteSize) * meshFormat->vertexByteSize == vertexAlloc.offset);

    Primitive prim{
            .meshFormat = *meshFormat,
            .baseVertex = vertexAlloc.offset / meshFormat->vertexByteSize,
            .vertexCount = vertexCount,
            .baseIndex = indexAlloc.offset / indexByteSize,
            .indexCount = indexCount,
    };

    return prim;
}

g2::gfx::Mesh g2::gfx::addMesh(UploadQueue *uploadQueue, MeshBuffer *meshBuffer, const g2::gfx::MeshData *meshData) {

    Mesh mesh;

    for (auto primitive : *meshData->primitives()) {
        MeshFormat format{
                .vertexByteSize = primitive->vertexAlignment(),
                .indexType = VK_INDEX_TYPE_UINT32,
        };

        //Todo actually append primitives
        mesh.primitives.push_back(addMeshPrimitive(uploadQueue, meshBuffer, &format, (void *) primitive->vertexData()->data(),
                       primitive->vertexCount(), (void *) primitive->indices()->data(), primitive->indices()->size()));
    }

    return mesh;
}

g2::AssetAddResult g2::gfx::MeshAssetManager::add_asset(std::span<const char> data) {
    auto meshData = g2::gfx::GetMeshData(data.data());

    auto index = nextMeshIndex++;

    meshes.emplace(index, addMesh(uploadQueue, meshBuffer, meshData));

    auto& mesh = meshes[index];

    std::vector<AssetReferencePatch> patches;
    patches.reserve(meshData->primitives()->size());
    int primIndex = 0;
    for(auto prim : *meshData->primitives()) {
        patches.push_back(AssetReferencePatch{
            .name = prim->material()->c_str(),
            .index = &mesh.primitives[primIndex++].material,
        });
    }


    return AssetAddResult{
            .index = index,
            .patches = std::move(patches),
    };

}

const char *g2::gfx::MeshAssetManager::ext() {
    return ".g2mesh";
}
