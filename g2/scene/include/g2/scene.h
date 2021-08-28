//
// Created by felix on 28/08/2021.
//

#ifndef G2_SCENE_H
#define G2_SCENE_H

#include <g2/ecs/registry.h>
#include <g2/assets/asset_registry.h>

namespace g2 {

    struct SceneAssetmanager : public IAssetManager {

        ecs::Registry* registry;

        AssetAddResult add_asset(std::span<const char> data) override;

        const char *ext() override;

    };

}

#endif //G2_SCENE_H
