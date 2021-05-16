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

    auto gfx = g2::gfx::Instance(gfxConfig);
    

    g2::AssetRegistry assetRegistry;

    for(auto m : gfx.getAssetManagers()) {
        assetRegistry.registerManager(m);
    }

    assetRegistry.includePath("assets");

    auto mesh = assetRegistry.getAssetIndex("assets/DamagedHelmet/DamagedHelmet.gltf/mesh_helmet_LP_13930damagedHelmet.g2mesh");

    auto mesh2 = assetRegistry.getAssetIndex("assets/FlightHelmet/FlightHelmet.gltf/LeatherParts_low.g2mesh");

    g2::gfx::DrawItem drawItems[]{
        {.mesh = mesh},
        {.mesh = mesh2},
        {.mesh = assetRegistry.getAssetIndex("assets/Sponza/Sponza.gltf/sponza.g2mesh")},
    };

    float r = 0;

    g2::gfx::Transform camera{};
    camera.pos = {0, 0, -1};
    camera.scale = 1;
    camera.orientation = glm::quatLookAt(glm::vec3(0, 0, -1), glm::vec3(0,-1,0));

    while (!app.shouldClose()) {
        app.pollEvents();
        gfx.setFramebufferExtent(app.getWindowSize());

        float dt = 0.0015;

        float movSpd = 0.6;

        if (app.inputState.keyDown(g2::KEYS::W)) {
            camera.pos += camera.orientation * glm::vec3(0,0,-1) * dt * movSpd;
        }
        if (app.inputState.keyDown(g2::KEYS::S)) {
            camera.pos += camera.orientation * glm::vec3(0,0,1) * dt * movSpd;
        }
        if (app.inputState.keyDown(g2::KEYS::A)) {
            camera.pos += camera.orientation * glm::vec3(-1, 0, 0) * dt * movSpd;
        }
        if (app.inputState.keyDown(g2::KEYS::D)) {
            camera.pos += camera.orientation * glm::vec3(1, 0, 0) * dt * movSpd;
        }
        if (app.inputState.keyDown(g2::KEYS::E)) {
            camera.orientation *= glm::quat(glm::vec3(0, -1, 0) * dt);
        }
        if (app.inputState.keyDown(g2::KEYS::Q)) {
            camera.orientation *= glm::quat(glm::vec3(0, 1, 0) * dt);
        }
        if (app.inputState.keyDown(g2::KEYS::SPACE)) {
            camera.pos += camera.orientation * glm::vec3(0, -1, 0) * dt * movSpd;
        }
        if (app.inputState.keyDown(g2::KEYS::LEFT_CONTROL)) {
            camera.pos += camera.orientation * glm::vec3(0, 1, 0) * dt * movSpd;
        }


        g2::gfx::Transform transforms[]{
                {
                        .pos = {1, 0, 0},
                        .scale = 1.0f,
                        .orientation = glm::quat(glm::vec3(0, 0, r)),
                },
                {
                        .pos = {-1, 0, 0},
                        .scale = 0.8f,
                        .orientation = glm::quat(),
                },
                {
                    .pos = {0,0,0},
                    .scale = 0.02f,
                    .orientation = glm::quat(),
                }
        };

        gfx.draw(drawItems, transforms, camera);

        r += 0.0001;
    }

}