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

    if (matDef->albedo()) {
        patches.push_back({.name = matDef->albedo()->c_str(), .index = &mat.albedoImage,});
    } else {
        mat.albedoImage = UINT32_MAX;
    }

    if (matDef->normal()) {
        patches.push_back({.name = matDef->normal()->c_str(), .index = &mat.normalImage,});
    } else {
        mat.normalImage = UINT32_MAX;
    }

    if (matDef->metallicRoughness()) {
        patches.push_back({.name = matDef->metallicRoughness()->c_str(), .index = &mat.metallicRoughnessImage,});
    } else {
        mat.metallicRoughnessImage = UINT32_MAX;
    }

    if (matDef->emissive()) {
        patches.push_back({.name = matDef->emissive()->c_str(), .index = &mat.emissiveImage,});
    } else {
        mat.emissiveImage = UINT32_MAX;
    }

    if (matDef->occlusion()) {
        patches.push_back({.name = matDef->occlusion()->c_str(), .index = &mat.occlusionImage,});
    } else {
        mat.occlusionImage = UINT32_MAX;
    }

    return AssetAddResult {
        .index = static_cast<uint32_t>(index),
        .patches = std::move(patches),
    };

}

const char *g2::gfx::MaterialAssetManager::ext() {
    return ".g2mat";
}
