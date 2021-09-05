//
// Created by felix on 23/04/2021.
//

#include "mesh.h"
#include <cstring>
#include <cassert>

namespace g2::gfx {

    void initMeshBuffer(VmaAllocator allocator, MeshBuffer *meshBuffer) {

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
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo indexAllocationInfo{
                .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        createLinearBuffer(allocator, &indexBufferInfo, &indexAllocationInfo, &meshBuffer->indexBuffer);
    }

    void destroyMeshBuffer(VmaAllocator allocator, g2::gfx::MeshBuffer *meshBuffer) {
        vmaDestroyBuffer(allocator, meshBuffer->vertexBuffer.buffer, meshBuffer->vertexBuffer.allocation);
        vmaDestroyBuffer(allocator, meshBuffer->indexBuffer.buffer, meshBuffer->indexBuffer.allocation);
    }

    static size_t uploadData(UploadQueue* uploadQueue, LinearBuffer& buffer, const flatbuffers::Vector<uint8_t>* vector) {
        auto alloc = allocateFromLinearBuffer(&buffer, vector->size(), sizeof(uint32_t));
        assert(alloc.size);
        uploadQueue->addJob(UploadJob{
                .priority = UINT16_MAX,
                .source = UploadSource {
                        .data = std::span((char*) vector->data(), vector->size()),
                        .compressed = false,
                },
                .target = BufferUploadTarget {
                        .targetBuffer = buffer.buffer,
                        .offset = alloc.offset,
                }
        });
        return alloc.offset;
    }

    static Primitive addMeshPrimitive(g2::gfx::UploadQueue *uploadQueue, g2::gfx::MeshBuffer *meshBuffer,
                                               const g2::gfx::MeshPrimitive *primitive) {


        auto positionOffset = uploadData(uploadQueue, meshBuffer->vertexBuffer, primitive->positionData());
        auto normalOffset = uploadData(uploadQueue, meshBuffer->vertexBuffer, primitive->normalData());
        auto texcoordOffset = uploadData(uploadQueue, meshBuffer->vertexBuffer, primitive->texcoordData());
        auto tangentOffset = uploadData(uploadQueue, meshBuffer->vertexBuffer, primitive->tangentData());
        auto bitangentOffset = uploadData(uploadQueue, meshBuffer->vertexBuffer, primitive->bitangentData());

        auto indexAlloc = allocateFromLinearBuffer(&meshBuffer->indexBuffer, primitive->indices()->size() * sizeof(uint32_t), 4);
        assert(indexAlloc.size);
        uploadQueue->addJob(UploadJob{
                .priority = UINT16_MAX,
                .source = UploadSource{
                        .data = std::span((char *) primitive->indices()->data(), primitive->indices()->size() * sizeof(uint32_t)),
                        .compressed = false,
                },
                .target = BufferUploadTarget{
                        .targetBuffer = meshBuffer->indexBuffer.buffer,
                        .offset = indexAlloc.offset
                },
        });

        std::vector<Meshlet> meshlets(primitive->meshlets()->size());
        for(int i = 0; i < primitive->meshlets()->size(); i++) {
            auto m = primitive->meshlets()->Get(i);
            meshlets[i].vertexOffset = m->vertexOffset();
            meshlets[i].triangleCount =  m->triangleCount();
            meshlets[i].triangleOffset = m->triangleOffset();
            meshlets[i].center = {m->center().x(), m->center().y(), m->center().z()};
            meshlets[i].radius = m->radius();
            meshlets[i].coneApex = {m->coneApex().x(), m->coneApex().y(), m->coneApex().z()};
            meshlets[i].coneAxis = {m->coneAxis().x(), m->coneAxis().y(), m->coneAxis().z()};
            meshlets[i].coneCutoff = m->coneCutoff();
        }

        assert(normalOffset % sizeof(uint32_t) == 0);

        Primitive prim {
                .positionOffset = positionOffset / sizeof(uint32_t),
                .normalOffset = normalOffset / sizeof(uint32_t),
                .texcoordOffset = texcoordOffset / sizeof(uint32_t),
                .tangentOffset = tangentOffset / sizeof(uint32_t),
                .bitangentOffset = bitangentOffset / sizeof(uint32_t),
                .baseIndex = indexAlloc.offset / sizeof(uint32_t),
                .material = 0,
                .meshlets = std::move(meshlets),
        };

        return prim;
    }

    Mesh addMesh(UploadQueue *uploadQueue, MeshBuffer *meshBuffer, const g2::gfx::MeshData *meshData) {

        Mesh mesh;

        for (auto primitive : *meshData->primitives()) {
            mesh.primitives.push_back( addMeshPrimitive(uploadQueue, meshBuffer, primitive) );
        }

        return mesh;
    }

    AssetAddResult  MeshAssetManager::add_asset(std::span<const char> data) {
        auto meshData = GetMeshData(data.data());

        auto index = nextMeshIndex++;

        meshes.emplace(index, addMesh(uploadQueue, meshBuffer, meshData));

        auto &mesh = meshes[index];

        std::vector<AssetReferencePatch> patches;
        patches.reserve(meshData->primitives()->size());
        int primIndex = 0;
        for (auto prim : *meshData->primitives()) {
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

}