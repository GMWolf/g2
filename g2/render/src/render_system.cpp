//
// Created by felix on 28/08/2021.
//

#include "render.h"
#include <g2/core/Transform.h>
#include <g2/ecs/query.h>
#include <g2/ecs/view.h>
#include <iostream>
namespace g2 {

    ecs::id_t c_meshRender;
    ecs::id_t c_camera;

    void registerRenderComponents(ecs::Registry &registry) {
        c_meshRender = registry.registerComponent<MeshRender>();
        c_camera = registry.registerComponent<Camera>();
    }

    static bool printFrame = true;

    static void renderForCamera(gfx::Instance &gfx, ecs::Registry &ecs, Transform camera) {
        std::vector<g2::Transform> transforms;
        std::vector<g2::gfx::DrawItem> drawItems;

        ecs::Query drawItemQuery;
        drawItemQuery.components = {c_transform, c_meshRender};

        if (printFrame ) {
            std::cout << "frame" << std::endl;
        }

        for(auto chunk : ecs::query(ecs, drawItemQuery)) {
            for(auto[transform, meshRender] : ecs::ChunkView<Transform, MeshRender>(chunk, c_transform, c_meshRender)) {

                transforms.push_back(transform);
                drawItems.push_back(gfx::DrawItem {
                    .mesh = meshRender.meshIndex
                });

                if (printFrame)
                    std::cout << "mesh: " << meshRender.meshIndex << std::endl;
            }
        }

        printFrame = false;

        gfx.draw(drawItems, transforms, camera);
    }

    void render(gfx::Instance &gfx, ecs::Registry &ecs) {

        ecs::Query cameraQuery;
        cameraQuery.components = {c_camera, c_transform};

        for(auto chunk : ecs::query(ecs, cameraQuery)) {
            for(auto[transform] : ecs::ChunkView<Transform>(chunk, c_transform)) {
                renderForCamera(gfx, ecs, transform);
            }
        }
    }

}
