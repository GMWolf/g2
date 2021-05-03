//
// Created by felix on 25/04/2021.
//

#include <iostream>
#include <span>
#include <cstring>
#include <algorithm>

#include <filesystem>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <cstdint>
#include <cassert>

#include <flatbuffers/flatbuffers.h>
#include <g2/gfx/mesh_generated.h>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;
namespace fb = flatbuffers;

struct Vertex {
    float positions[4];
    float normals[4];
    float texcoords[4];
};

static const char* GLTF_ATTRIBUTE_POSITION = "POSITION";
static const char* GLTF_ATTRIBUTE_NORMAL = "NORMAL";
static const char* GLTF_ATTRIBUTE_TEXCOORD = "TEXCOORD_0";


int main(int argc, char* argv[]) {

    if (argc != 4) {
        std::cerr << "expected 3 arguments. usage: input name output\n";
        return -1;
    }

    const fs::path input = argv[1];
    const char* meshName = argv[2];
    const fs::path output = argv[3];

    const fs::path directory = input.parent_path();

    cgltf_options options = {};
    cgltf_data* data{};
    cgltf_result result = cgltf_parse_file(&options, input.c_str(), &data);

    if(result != cgltf_result_success) {
        std::cerr << "Error loading file: " << input << "\n";
        return 1;
    }

    result = cgltf_load_buffers(&options, data, input.c_str());
    if(result != cgltf_result_success) {
        std::cerr << "Error loading buffers for " << input << "error code: " << result << "\n";
        return 1;
    }

    auto mesh = std::find_if(data->meshes, data->meshes + data->meshes_count, [meshName](const cgltf_mesh& mesh) {
        return strcmp(mesh.name, meshName) == 0;
    });

    if (mesh == data->meshes + data->meshes_count) {
        std::cerr << "Could not fine mesh " << meshName << " in " << input << "\n";
        return 1;
    }

    fb::FlatBufferBuilder fbb(1024 * 1024);

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
    fbb.Finish(fbMesh);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();

        fs::create_directories(output.parent_path());

        std::ofstream ofs(output.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)buf, buf_size);
    }

    {
        std::ofstream ofs(output.string() + ".d");
        std::regex whitespaceRegex("\\s");

        ofs << output.c_str() << " :";

        for(cgltf_buffer& buffer : std::span(data->buffers, data->buffers_count)) {
            ofs << " " << std::regex_replace((directory / buffer.uri).c_str(), whitespaceRegex, "\\$&");
        }
    }



    cgltf_free(data);

    return 0;
}