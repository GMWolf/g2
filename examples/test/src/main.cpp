//
// Created by felix on 13/04/2021.
//

#include <iostream>
#include <g2/application.h>
#include <g2/gfx_instance.h>
#include <fstream>
#include <vector>
#include <g2/gfx/pipeline_generated.h>
#include <g2/gfx/mesh_generated.h>
#include <g2/gfx/material_generated.h>
#include <g2/assets/asset_registry.h>
#include <unistd.h>
#include <g2/render/render.h>
#include <g2/ecs/registry.h>
#include <g2/core/core.h>
#include "camera.h"

int main() {
    g2::ApplicationConfiguration appConfig{
            .width = 1280,
            .height = 720,
            .title = "Application"
    };

    g2::gfx::init();

    auto app = g2::Application(appConfig);

    g2::gfx::InstanceConfig gfxConfig{
            .application = &app,
            .vkExtensions = g2::getVkExtensions(),
    };

    g2::gfx::Instance gfx(gfxConfig);

    g2::AssetRegistry assetRegistry;

    for(auto m : gfx.getAssetManagers()) {
        assetRegistry.registerManager(m);
    }

    assetRegistry.includePath("assets");

    g2::ecs::Registry ecs;

    g2::registerRenderComponents(ecs);
    g2::registerCoreComponents(ecs);

    g2::ecs::id_t helmet = ecs.create({g2::c_transform, g2::c_meshRender});
    ecs.get<g2::Transform>(helmet, g2::c_transform) = {
            .pos = {1, 1, -2},
            .scale = 1.0f,
            .orientation = glm::quat(glm::vec3(0, 0, 0))
    };
    ecs.get<g2::MeshRender>(helmet, g2::c_meshRender) = {
            .meshIndex = assetRegistry.getAssetIndex("assets/DamagedHelmet/DamagedHelmet.gltf/mesh_helmet_LP_13930damagedHelmet.g2mesh")
    };

    g2::ecs::id_t flightHelmet = ecs.create({g2::c_transform, g2::c_meshRender});
    ecs.get<g2::Transform>(flightHelmet, g2::c_transform) = {
            .pos = {1, 0, 2},
            .scale = 1.0f,
            .orientation = glm::quat(glm::vec3(0, 0, 0)),
    };
    ecs.get<g2::MeshRender>(flightHelmet, g2::c_meshRender) = {
            .meshIndex = assetRegistry.getAssetIndex("assets/FlightHelmet/FlightHelmet.gltf/LeatherParts_low.g2mesh")
    };

    g2::ecs::id_t sponza = ecs.create({g2::c_transform, g2::c_meshRender});
    ecs.get<g2::Transform>(sponza, g2::c_transform) = {
            .pos = {0,0,0},
            .scale = 0.02f,
            .orientation = glm::quat(),
    };
    ecs.get<g2::MeshRender>(sponza, g2::c_meshRender) = {
            .meshIndex = assetRegistry.getAssetIndex("assets/Sponza/Sponza.gltf/sponza.g2mesh")
    };


    g2::ecs::id_t camera = ecs.create({g2::c_transform, g2::c_camera, c_fpsController});
    ecs.get<g2::Transform>(camera, g2::c_transform) = {
            .pos = {0, 0, -1},
            .scale = 1,
            .orientation = glm::quatLookAt(glm::vec3(0, 0, -1), glm::vec3(0,-1,0)),
    };
    ecs.get<FPSController>(camera, c_fpsController) = {
            .movSpd = 2.5
    };

    double lastTime = app.getTime();

    while (!app.shouldClose()) {
        app.pollEvents();
        gfx.setFramebufferExtent(app.getWindowSize());

        double time = app.getTime();
        float dt = (float)time - lastTime;
        lastTime = time;

        updateCameras(ecs, app.inputState, dt);

        g2::render(gfx, ecs);

    }

}