//
// Created by felix on 03/05/2021.
//

#include "mesh_compiler.h"
#include <g2/gfx/mesh_generated.h>
#include <span>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtx/component_wise.hpp>

namespace fb = flatbuffers;

static const char* GLTF_ATTRIBUTE_POSITION = "POSITION";
static const char* GLTF_ATTRIBUTE_NORMAL = "NORMAL";
static const char* GLTF_ATTRIBUTE_TEXCOORD = "TEXCOORD_0";

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};


struct PackedVertex {
    glm::u16vec3 position;
    glm::u8vec2 normals;

    glm::u16vec2 texcoords;
};
static_assert(sizeof(PackedVertex) == 6 * sizeof(uint16_t));

glm::vec2 encode_oct(const glm::vec3& va) {

    glm::vec3 v = glm::normalize(va);

    glm::vec2 p = glm::vec2(v.x, v.y) * (1.0f / (std::abs(v.x) + std::abs(v.y) + std::abs(v.z)));
    if (v.z <= 0.0) {
        glm::vec2 snz {
            p.x >= 0.0 ? 1.0 : -1.0,
            p.y >= 0.0 ? 1.0 : -1.0,
        };

        return (glm::vec2(1.0) - glm::abs(glm::vec2(p.y, p.x))) * snz;
    } else {
        return p;
    }
}

PackedVertex packVertex(const Vertex& vertex) {

    PackedVertex packed{};

    packed.position = glm::packHalf(vertex.position);
    packed.normals = glm::packSnorm<int8_t>(encode_oct(vertex.normal));
    packed.texcoords = glm::packHalf(vertex.texcoords);
    return packed;

}


std::vector<uint8_t> compileMesh(const cgltf_mesh *mesh) {

    fb::FlatBufferBuilder fbb(1024);

    std::vector<fb::Offset<g2::gfx::MeshPrimitive>> fbPrimitives;

    for(const cgltf_primitive& primitive : std::span(mesh->primitives, mesh->primitives_count)) {
        std::vector<uint32_t> indices(primitive.indices->count);
        std::vector<Vertex> vertices(primitive.attributes[0].data->count);

        for(int index = 0; index < primitive.indices->count; index++) {
            indices[index] = cgltf_accessor_read_index(primitive.indices, index);
        }

        for(const cgltf_attribute& attribute : std::span(primitive.attributes, primitive.attributes_count)) {
            assert(attribute.data->count == vertices.size());

            if (strcmp(attribute.name, GLTF_ATTRIBUTE_POSITION) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &vertices[vertexIndex].position.x, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_NORMAL) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &vertices[vertexIndex].normal.x, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_TEXCOORD) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &vertices[vertexIndex].texcoords.x, 2);
                }
            }
        }

        std::vector<PackedVertex> packedVertices(vertices.size());
        std::transform(vertices.begin(), vertices.end(), packedVertices.begin(), packVertex);

        auto fbMaterialName = fbb.CreateString(std::string(primitive.material->name) + ".g2mat");

        auto fbIndices = fbb.CreateVector(indices);
        auto fbVertices = fbb.CreateVector(reinterpret_cast<uint8_t*>(packedVertices.data()), packedVertices.size() * sizeof(PackedVertex));

        fbPrimitives.push_back(g2::gfx::CreateMeshPrimitive(fbb, fbIndices, fbVertices, packedVertices.size(), sizeof(PackedVertex), fbMaterialName));
    }

    auto fbMesh = g2::gfx::CreateMeshDataDirect(fbb, &fbPrimitives);
    g2::gfx::FinishMeshDataBuffer(fbb, fbMesh);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
