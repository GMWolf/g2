//
// Created by felix on 03/05/2021.
//

#include "mesh_compiler.h"
#include <g2/gfx/mesh_generated.h>
#include <span>

namespace fb = flatbuffers;

static const char* GLTF_ATTRIBUTE_POSITION = "POSITION";
static const char* GLTF_ATTRIBUTE_NORMAL = "NORMAL";
static const char* GLTF_ATTRIBUTE_TEXCOORD = "TEXCOORD_0";

struct Vertex {
    float positions[4];
    float normals[4];
    float texcoords[4];
};


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
                    cgltf_accessor_read_float(attribute.data, vertexIndex, vertices[vertexIndex].positions, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_NORMAL) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, vertices[vertexIndex].normals, 3);
                }
            } else if (strcmp(attribute.name, GLTF_ATTRIBUTE_TEXCOORD) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    cgltf_accessor_read_float(attribute.data, vertexIndex, vertices[vertexIndex].texcoords, 2);
                }
            }
        }

        auto fbIndices = fbb.CreateVector(indices);
        auto fbVertices = fbb.CreateVector(reinterpret_cast<uint8_t*>(vertices.data()), vertices.size() * sizeof(Vertex));

        fbPrimitives.push_back(g2::gfx::CreateMeshPrimitive(fbb, fbIndices, fbVertices, vertices.size(), sizeof(Vertex)));
    }

    auto fbMesh = g2::gfx::CreateMeshDataDirect(fbb, &fbPrimitives);
    g2::gfx::FinishMeshDataBuffer(fbb, fbMesh);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
