//
// Created by felix on 28/08/2021.
//

#include <span>
#include <g2/assets/asset_registry.h>
#include <scene.h>
#include <g2/scene/scene_generated.h>
#include <g2/core/Transform.h>
#include <g2/render/render.h>
#include <iostream>
namespace g2 {

    AssetAddResult SceneAssetmanager::add_asset(std::span<const char> data) {

        auto sceneData = GetScene(data.data());

        AssetAddResult result{
            .index = 0,
            .patches = {},
            };

        int i = 0;
        for(auto node : *sceneData->nodes()) {

            g2::ecs::Type type {c_transform};

            if (node->mesh()) {
                type.components.push_back(c_meshRender);
            }

            assert(node->name());
            std::string name = node->name()->str();
            if (name == "Camera") {
                type.components.push_back(c_camera);
            }

            std::cout << name << std::endl;

            auto entity = registry->create(type);
            registry->get<g2::Transform>(entity, c_transform) = g2::Transform{
                    .pos = {
                            node->transform()->pos()->Get(0),
                            node->transform()->pos()->Get(1),
                            node->transform()->pos()->Get(2),
                    },
                    .scale = node->transform()->scale(),// * 0.02f,
                    .orientation = {
                            node->transform()->rot()->Get(3),
                            node->transform()->rot()->Get(0),
                            node->transform()->rot()->Get(1),
                            node->transform()->rot()->Get(2),
                    }
            };


            if (node->mesh()) {
                registry->get<g2::MeshRender>(entity, c_meshRender).meshIndex = 0;
                result.patches.push_back(AssetReferencePatch{
                        .name = node->mesh()->c_str(),
                        .index = &registry->get<g2::MeshRender>(entity, c_meshRender).meshIndex,
                });
            }

        }

        return result;
    }

    const char *SceneAssetmanager::ext() {
        return ".scene";
    }

}
