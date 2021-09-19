//
// Created by felix on 02/06/2021.
//

#include "effect.h"
#include <g2/gfx/effect_generated.h>


g2::AssetAddResult g2::gfx::EffectAssetManager::add_asset(std::span<const char> data) {

    auto effectDef = g2::gfx::GetEffectDef(data.data());

    std::vector<AssetReferencePatch> patches;

    auto index = nextMaterialId++;
    effects.emplace(index);


    for(auto pass : *effectDef->passes()) {
        effects[index].passes.push_back(Effect::Pass {
            .passId = pass->passId()->c_str(),
            .pipelineIndex = UINT32_MAX,
        });
    }

    for(int i = 0; i < effectDef->passes()->size(); i++) {
        patches.push_back(AssetReferencePatch{
                .name = effectDef->passes()->Get(i)->pipeline()->c_str(),
                .index = &effects[index].passes[i].pipelineIndex,
        });
    }

    return g2::AssetAddResult {
        .index = index,
        .patches = std::move(patches),
    };
}

const char *g2::gfx::EffectAssetManager::ext() {
    return ".g2fx";
}

uint32_t g2::gfx::Effect::getPipelineIndex(const char *name) {

    for(auto pass : passes) {
        if (strcmp(name, pass.passId) == 0) {
            return pass.pipelineIndex;
        }
    }

    return 0;
}
