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

    fs::path albedoUri;
    const char* c_albedoUri = nullptr;
    if(mat->pbr_metallic_roughness.base_color_texture.texture) {
        albedoUri = (fs::path("..") / mat->pbr_metallic_roughness.base_color_texture.texture->image->uri).concat(".g2img");
        c_albedoUri = albedoUri.c_str();
    }
    g2::gfx::Vec3 albedoFactor(
            mat->pbr_metallic_roughness.base_color_factor[0],
            mat->pbr_metallic_roughness.base_color_factor[1],
            mat->pbr_metallic_roughness.base_color_factor[2]
    );

    fs::path metallicRoughnessUri;
    const char* c_metallicRoughnessUri = nullptr;
    if(mat->pbr_metallic_roughness.metallic_roughness_texture.texture) {
        metallicRoughnessUri = (fs::path("..") / mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri).concat(".g2img");
        c_metallicRoughnessUri = metallicRoughnessUri.c_str();
    }
    float metallicFactor = mat->pbr_metallic_roughness.metallic_factor;
    float roughnessFactor = mat->pbr_metallic_roughness.roughness_factor;


    fs::path normalUri;
    const char* c_normalUri = nullptr;
    if(mat->normal_texture.texture) {
        normalUri = (fs::path("..") / mat->normal_texture.texture->image->uri).concat(".g2img");
        c_normalUri = normalUri.c_str();
    }


    fs::path occlusionUri;
    const char* c_occlusionUri = nullptr;
    if(mat->occlusion_texture.texture) {
        occlusionUri = (fs::path("..") / mat->occlusion_texture.texture->image->uri).concat(".g2img");
        c_occlusionUri = occlusionUri.c_str();
    }

    fs::path emissiveUri;
    const char* c_emissiveUri = nullptr;
    if (mat->emissive_texture.texture) {
        emissiveUri = (fs::path("..") /mat->emissive_texture.texture->image->uri).concat(".g2img");
        c_emissiveUri = emissiveUri.c_str();
    }
    g2::gfx::Vec3 emissiveFactor (
            mat->emissive_factor[0],
            mat->emissive_factor[1],
            mat->emissive_factor[2]
            );

    auto m = g2::gfx::CreateMaterialDefDirect(fbb, mat->name, c_albedoUri,
                                                         &albedoFactor,
                                                         c_metallicRoughnessUri,
                                                         metallicFactor,
                                                         roughnessFactor,
                                                         c_normalUri,
                                                         c_occlusionUri,
                                                         c_emissiveUri,
                                                         &emissiveFactor);

    g2::gfx::FinishMaterialDefBuffer(fbb, m);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
