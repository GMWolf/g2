//
// Created by felix on 01/05/2021.
//

#ifndef G2_MATERIAL_H
#define G2_MATERIAL_H

#include <cstdint>
#include <g2/assets/asset_registry.h>
#include <g2/hat.h>

namespace g2::gfx {

    struct Material {
        static const uint32_t invalidImage = UINT32_MAX;

        uint32_t albedoImage;
        uint32_t normalImage;
        uint32_t metallicRoughnessImage;
        uint32_t occlusionImage;
        uint32_t emissiveImage;
        uint32_t pad0, pad1, pad2;

        float albedoMetallicFactor[4];
        float emissiveRoughnessFactor[4];
    };


    struct MaterialAssetManager : public IAssetManager {

        //g2::hat<Material> materials;
        size_t nextMaterialId = 0;

        Material* materials = nullptr;

        AssetAddResult add_asset(std::span<const char> data) override;

        const char *ext() override;
    };

}


#endif //G2_MATERIAL_H
