//
// Created by felix on 13/04/2021.
//

#include <iostream>
#include <g2/application.h>
#include <g2/gfx_instance.h>
#include <fstream>
#include <vector>
#include <g2/assets/asset_registry.h>
#include <unistd.h>
#include <g2/render/render.h>
#include <g2/ecs/registry.h>
#include <g2/core/core.h>
#include "camera.h"
#include <g2/scene.h>

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

    g2::SceneAssetmanager sceneAssetmanager;
    g2::ecs::Registry ecs;
    g2::registerRenderComponents(ecs);
    g2::registerCoreComponents(ecs);
    c_fpsController = ecs.registerComponent<FPSController>();
    sceneAssetmanager.registry = &ecs;

    for(auto m : gfx.getAssetManagers()) {
        assetRegistry.registerManager(m);
    }

    assetRegistry.registerManager(&sceneAssetmanager);

    assetRegistry.includePath("assets");


    g2::ecs::id_t camera = ecs.create({g2::c_transform, g2::c_camera, c_fpsController});
    ecs.get<g2::Transform>(camera, g2::c_transform) = {
            .pos = {0, 0, 0},
            .scale = 1,
            .orientation = glm::quatLookAt(glm::vec3(1, 0, 0), glm::vec3(0,-1,0)),
    };
    ecs.get<FPSController>(camera, c_fpsController) = {
            .movSpd = 3
    };

    double lastTime = app.getTime();

    while (!app.shouldClose()) {
        app.pollEvents();
        gfx.setFramebufferExtent(app.getWindowSize());

        double time = app.getTime();
        float dt = (float)time - lastTime;
        lastTime = time;

        if (app.inputState.keyPressed(g2::KEYS::N_1)) {
            gfx.setScriptIndex(0);
        }
        if (app.inputState.keyPressed(g2::KEYS::N_2)) {
            gfx.setScriptIndex(1);
        }

        updateCameras(ecs, app.inputState, dt);

        g2::render(gfx, ecs);

    }

}