//
// Created by felix on 01/05/2021.
//

#include "material.h"
#include <g2/gfx/material_generated.h>


g2::AssetAddResult g2::gfx::MaterialAssetManager::add_asset(std::span<char> data) {

    auto matDef = g2::gfx::GetMaterialMap(data.data());

    auto index = nextMaterialId++;

    materials.emplace(index);

    auto& mat = materials[index];

    //TODO modify asset system to support sets of assets from single file

    AssetReferencePatch patches[] = {
            {
                    .name = matDef->materials()->Get(0)->albedo()->c_str(),
                    .index = &mat.albedoImage,
            },
            {
                .name = matDef->materials()->Get(0)->normal()->c_str(),
                .index = &mat.normalImage,
            }
    };



    return AssetAddResult {
        .index = static_cast<uint32_t>(index),
        .patches = std::vector<AssetReferencePatch>(patches, patches + 2),
    };

}

const char *g2::gfx::MaterialAssetManager::ext() {
    return ".g2mat";
}
