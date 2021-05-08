//
// Created by felix on 03/05/2021.
//

#include <g2/gfx/material_generated.h>
#include "material_compiler.h"
#include <filesystem>


namespace fb = flatbuffers;
namespace fs = std::filesystem;

std::vector<uint8_t> compileMaterial(const cgltf_material *mat) {

    fb::FlatBufferBuilder fbb(1024);

    auto albedoUri = (fs::path("..") / mat->pbr_metallic_roughness.base_color_texture.texture->image->uri).concat(".g2img");
    auto metallicRoughnessUri =(fs::path("..") / mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri).concat(".g2img");
    auto normalUri = (fs::path("..") / mat->normal_texture.texture->image->uri).concat(".g2img");
    auto occlusionUri = (fs::path("..") /mat->occlusion_texture.texture->image->uri).concat(".g2img");
    auto emissiveUri = (fs::path("..") /mat->emissive_texture.texture->image->uri).concat(".g2img");

    auto m = g2::gfx::CreateMaterialDefDirect(fbb, mat->name, albedoUri.c_str(),
                                                         metallicRoughnessUri.c_str(),
                                                         normalUri.c_str(),
                                                         occlusionUri.c_str(),
                                                         emissiveUri.c_str());

    g2::gfx::FinishMaterialDefBuffer(fbb, m);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
