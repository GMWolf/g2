//
// Created by felix on 03/05/2021.
//

#include "mesh_compiler.h"
#include <g2/gfx/mesh_generated.h>
#include <span>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtx/component_wise.hpp>
#include <meshoptimizer.h>

namespace fb = flatbuffers;

static const char* GLTF_ATTRIBUTE_POSITION = "POSITION";
static const char* GLTF_ATTRIBUTE_NORMAL = "NORMAL";
static const char* GLTF_ATTRIBUTE_TEXCOORD = "TEXCOORD_0";


struct Meshlet {
    size_t positionsOffset; // offset in words to positions
    size_t normalsOffset; // offset in words to normals
    size_t texcoordsOffset; // offset in words to texcoords
};


glm::vec2 sign_not_zero(glm::vec2 v)
{
    return glm::vec2((v.x >= 0 ? 1.0 : -1.0),
                     (v.y >=0 ? 1.0 : -1.0));
}

glm::vec2 encode_oct(const glm::vec3& va) {

    glm::vec3 v = glm::normalize(va);

    glm::vec2 p = glm::vec2(v.x, v.y) * (1.0f / (std::abs(v.x) + std::abs(v.y) + std::abs(v.z)));

    return v.z <= 0 ? ((glm::vec2(1.0) - glm::abs(glm::vec2(p.y, p.x))) * sign_not_zero(p)) : p;
}


template<class T>
meshopt_Stream meshoptStream(const std::vector<T>& v) {
    meshopt_Stream stream{};
    stream.data = v.data();
    stream.size = sizeof(T);
    stream.stride = sizeof(T);
    return stream;
}


template<class T>
fb::Offset<fb::Vector<uint8_t>> createByteVector(fb::FlatBufferBuilder& fbb, const std::vector<T>& vec) {
    return fbb.template CreateVector((uint8_t*)vec.data(), vec.size() * sizeof(T));
}


std::vector<uint8_t> compileMesh(const cgltf_mesh *mesh) {

    fb::FlatBufferBuilder fbb(1024);

    std::vector<fb::Offset<g2::gfx::MeshPrimitive>> fbPrimitives;

    for(const cgltf_primitive& primitive : std::span(mesh->primitives, mesh->primitives_count)) {
        std::vector<uint32_t> indices(primitive.indices->count);
        std::vector<glm::vec3> positions(primitive.attributes[0].data->count);
        std::vector<glm::vec3> normals(primitive.attributes[0].data->count);
        std::vector<glm::vec2> texcoords(primitive.attributes[0].data->count);

        for(int index = 0; index < primitive.indices->count; index++) {
            indices[index] = cgltf_accessor_read_index(primitive.indices, index);
        }

        for(const cgltf_attribute& attribute : std::span(primitive.attributes, primitive.attributes_count)) {
            assert(attribute.data->count == positions.size());

            if (strcmp(attribute.name, GLTF_ATTRIBUTE_POSITION) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &positions[vertexIndex].x, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_NORMAL) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &normals[vertexIndex].x, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_TEXCOORD) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &texcoords[vertexIndex].x, 2);
                }
            }
        }


        { // Remap
            meshopt_Stream streams[]{
                    meshoptStream(positions),
                    meshoptStream(normals),
                    meshoptStream(texcoords)
            };

            std::vector<uint32_t> remap(indices.size());
            uint32_t remapVertexCount = meshopt_generateVertexRemapMulti(remap.data(), indices.data(), indices.size(), positions.size(), streams,
                                             3);
            meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(), remap.data());
            meshopt_remapVertexBuffer(positions.data(), positions.data(), positions.size(), sizeof(glm::vec3), remap.data());
            positions.resize(remapVertexCount);
            meshopt_remapVertexBuffer(normals.data(), normals.data(), normals.size(), sizeof(glm::vec3), remap.data());
            normals.resize(remapVertexCount);
            meshopt_remapVertexBuffer(texcoords.data(), texcoords.data(), texcoords.size(), sizeof(glm::vec2), remap.data());
            texcoords.resize(remapVertexCount);
        }

        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), positions.size());



        const size_t maxVertices = 64;
        const size_t maxTriangles = 124;
        float coneWeight = 0.5f;
        size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);
        std::vector<meshopt_Meshlet> meshlets(maxMeshlets);

        std::vector<uint32_t> meshletVertices(maxMeshlets * maxVertices);
        std::vector<uint8_t> meshletTriangles(maxMeshlets * maxTriangles * 3);

        size_t meshletCount = meshopt_buildMeshlets(meshlets.data(), meshletVertices.data(), meshletTriangles.data(),
                                                    indices.data(), indices.size(), &positions[0].x, positions.size(), sizeof(glm::vec3), maxVertices, maxTriangles, coneWeight);
        meshlets.resize(meshletCount);
        meshletVertices.resize(meshlets.back().vertex_offset + meshlets.back().vertex_count);
        meshletTriangles.resize(meshlets.back().triangle_offset + ((meshlets.back().triangle_count * 3 + 3) & ~3));

        std::vector<uint32_t> packedIndices(meshletTriangles.size());
        std::transform(meshletTriangles.begin(), meshletTriangles.end(), packedIndices.begin(), [](uint8_t index) {
            return index;
        });

        std::vector<glm::vec3> packedPositions(meshletVertices.size());
        std::transform(meshletVertices.begin(), meshletVertices.end(), packedPositions.begin(), [&positions](uint32_t vertex) {
            return positions[vertex];
        } );

        std::vector<glm::i16vec2> packedNormals(meshletVertices.size());
        std::transform(meshletVertices.begin(), meshletVertices.end(), packedNormals.begin(), [&normals](uint32_t vertex) {
            return glm::packSnorm<int16_t>(encode_oct(normals[vertex]));
        });



        std::vector<glm::u16vec2> packedTexcoords(meshletVertices.size());
        std::transform(meshletVertices.begin(), meshletVertices.end(), packedTexcoords.begin(), [&texcoords](uint32_t vertex) {
            return glm::packHalf(texcoords[vertex]);
        });

        std::vector<g2::gfx::MeshletData> meshletDataVec;

        for(auto meshlet : meshlets) {

            meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset], &meshletTriangles[meshlet.triangle_offset],
                                                                 meshlet.triangle_count, &positions[0].x, positions.size(), sizeof(glm::vec3));

            meshletDataVec.push_back(g2::gfx::MeshletData(
                    g2::gfx::Vec3(bounds.center[0], bounds.center[1], bounds.center[2]),
                    bounds.radius,
                    g2::gfx::Vec3(bounds.cone_apex[0], bounds.cone_apex[1], bounds.cone_apex[2]),
                    g2::gfx::Vec3(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2]),
                    bounds.cone_cutoff,
                    meshlet.triangle_offset, meshlet.triangle_count, meshlet.vertex_offset));
        }


        auto fbPositions = createByteVector(fbb, packedPositions);
        auto fbNormals = createByteVector(fbb, packedNormals);
        auto fbTexcoords = createByteVector(fbb, packedTexcoords);

        auto fbMeshlets = fbb.CreateVectorOfStructs(meshletDataVec);

        auto fbMaterialName = fbb.CreateString(std::string(primitive.material->name) + ".g2mat");

        auto fbIndices = fbb.CreateVector(packedIndices);

        fbPrimitives.push_back(g2::gfx::CreateMeshPrimitive(fbb, fbIndices, fbPositions, fbNormals, fbTexcoords, fbMeshlets, fbMaterialName));
    }

    auto fbMesh = g2::gfx::CreateMeshDataDirect(fbb, &fbPrimitives);
    g2::gfx::FinishMeshDataBuffer(fbb, fbMesh);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
