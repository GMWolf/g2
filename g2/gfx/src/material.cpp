//
// Created by felix on 01/05/2021.
//

#include "material.h"
#include <g2/gfx/material_generated.h>


g2::AssetAddResult g2::gfx::MaterialAssetManager::add_asset(std::span<const char> data) {

    auto matDef = g2::gfx::GetMaterialDef(data.data());

    auto index = nextMaterialId++;

    //materials.emplace(index);

    auto& mat = materials[index];

    std::vector<AssetReferencePatch> patches;

    if (matDef->albedoTexture()) {
        patches.push_back({.name = matDef->albedoTexture()->c_str(), .index = &mat.albedoImage,});
    } else {
        mat.albedoImage = UINT32_MAX;
    }

    if (matDef->normalTexture()) {
        patches.push_back({.name = matDef->normalTexture()->c_str(), .index = &mat.normalImage,});
    } else {
        mat.normalImage = UINT32_MAX;
    }

    if (matDef->metallicRoughnessTexture()) {
        patches.push_back({.name = matDef->metallicRoughnessTexture()->c_str(), .index = &mat.metallicRoughnessImage,});
    } else {
        mat.metallicRoughnessImage = UINT32_MAX;
    }

    if (matDef->emissiveTexture()) {
        patches.push_back({.name = matDef->emissiveTexture()->c_str(), .index = &mat.emissiveImage,});
    } else {
        mat.emissiveImage = UINT32_MAX;
    }

    if (matDef->occlusionTexture()) {
        patches.push_back({.name = matDef->occlusionTexture()->c_str(), .index = &mat.occlusionImage,});
    } else {
        mat.occlusionImage = UINT32_MAX;
    }

    mat.albedoMetallicFactor[0] = matDef->albedoFactor()->x();
    mat.albedoMetallicFactor[1] = matDef->albedoFactor()->y();
    mat.albedoMetallicFactor[2] = matDef->albedoFactor()->z();
    mat.albedoMetallicFactor[3] = matDef->metallicFactor();
    mat.emissiveRoughnessFactor[0] = matDef->emissiveFactor()->x();
    mat.emissiveRoughnessFactor[1] = matDef->emissiveFactor()->y();
    mat.emissiveRoughnessFactor[2] = matDef->emissiveFactor()->z();
    mat.emissiveRoughnessFactor[3] = matDef->roughnessFactor();

    return AssetAddResult {
        .index = static_cast<uint32_t>(index),
        .patches = std::move(patches),
    };

}

const char *g2::gfx::MaterialAssetManager::ext() {
    return ".g2mat";
}
