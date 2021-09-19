//
// Created by felix on 02/06/2021.
//

#ifndef G2_EFFECT_H
#define G2_EFFECT_H


#include <cstdint>
#include <vector>
#include <g2/assets/asset_registry.h>
#include <g2/hat.h>

namespace g2::gfx {

    struct Effect {
        struct Pass {
            const char* passId;
            uint32_t pipelineIndex;
        };

        std::vector<Pass> passes;

        uint32_t getPipelineIndex(const char* name);
    };



    struct EffectAssetManager : public IAssetManager {
        hat<Effect, 64> effects;

        uint32_t nextMaterialId = 0;

        AssetAddResult add_asset(std::span<const char> data) override;
        const char *ext() override;
    };


}


#endif //G2_EFFECT_H
