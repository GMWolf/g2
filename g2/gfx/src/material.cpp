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

    AssetReferencePatch patches[] = {
            {.name = matDef->albedo()->c_str(), .index = &mat.albedoImage,},
            {.name = matDef->normal()->c_str(), .index = &mat.normalImage,},
            {.name = matDef->emissive()->c_str(), .index = &mat.emissiveImage,},
            {.name = matDef->metallicRoughness()->c_str(), .index = &mat.metallicRoughnessImage,},
            {.name = matDef->occlusion()->c_str(), .index = &mat.occlusionImage,},
    };


    return AssetAddResult {
        .index = static_cast<uint32_t>(index),
        .patches = std::vector<AssetReferencePatch>(patches, patches + 5),
    };

}

const char *g2::gfx::MaterialAssetManager::ext() {
    return ".g2mat";
}
