//
// Created by felix on 28/04/2021.
//

#include <iostream>
#include <filesystem>
#include <span>
#include <fstream>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <g2/gfx/material_generated.h>

namespace fs = std::filesystem;
namespace fb = flatbuffers;


int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "expected 2 arguments\n";
        return 1;
    }

    const fs::path input = argv[1];
    const fs::path output = argv[2];
    const fs::path directory = input.parent_path();
    const fs::path outputDir = output.parent_path();

    cgltf_options options = {};
    cgltf_data* data{};
    cgltf_result result = cgltf_parse_file(&options, input.c_str(), &data);

    if(result != cgltf_result_success) {
        std::cerr << "Error loading file: " << input << "\n";
        return 1;
    }

    fb::FlatBufferBuilder fbb(1024);

    std::vector<fb::Offset<g2::gfx::MaterialDef>> materials;

    for(auto& mat : std::span(data->materials, data->materials_count)) {
        auto albedoUri = (outputDir / mat.pbr_metallic_roughness.base_color_texture.texture->image->uri).concat(".ktx2");
        auto metallicRoughnessUri = (outputDir/ mat.pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri).concat(".ktx2");
        auto normalUri = (outputDir / mat.normal_texture.texture->image->uri).concat(".ktx2");
        auto occlusionUri = (outputDir / mat.occlusion_texture.texture->image->uri).concat(".ktx2");
        auto emissiveUri = (outputDir / mat.emissive_texture.texture->image->uri).concat(".ktx2");

        materials.push_back(g2::gfx::CreateMaterialDefDirect(fbb, mat.name, albedoUri.c_str(),
                                                             metallicRoughnessUri.c_str(),
                                                             normalUri.c_str(),
                                                             occlusionUri.c_str(),
                                                             emissiveUri.c_str()));
    }

    auto materialMap = g2::gfx::CreateMaterialMapDirect(fbb, &materials);

    fbb.Finish(materialMap);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();

        fs::create_directories(output.parent_path());

        std::ofstream ofs(output.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)buf, buf_size);
    }

    return 0;
}