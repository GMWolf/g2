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
    assetRegistry.includePath("assets");


    std::ifstream pipelineDefInput("assets/pipeline.json.g2ppln", std::ios::binary);
    std::vector<char> pipelineBytes((std::istreambuf_iterator<char>(pipelineDefInput)),
                                    (std::istreambuf_iterator<char>()));

    VkPipeline pipeline = gfx.createPipeline(g2::gfx::GetPipelineDef(pipelineBytes.data()));


    std::ifstream meshStream("assets/DamagedHelmet/DamagedHelmet.gltf.mesh_helmet_LP_13930damagedHelmet.g2mesh",
                             std::ios::binary);
    std::vector<char> meshBytes((std::istreambuf_iterator<char>(meshStream)),
                                (std::istreambuf_iterator<char>()));

    const g2::gfx::MeshData *meshData = g2::gfx::GetMeshData(meshBytes.data());

    auto mesh = gfx.addMesh(meshData);


    std::ifstream materialsStream("assets/DamagedHelmet/DamagedHelmet.gltf.g2mat",
                                  std::ios::binary);
    std::vector<char> materialsBytes((std::istreambuf_iterator<char>(materialsStream)),
                                     (std::istreambuf_iterator<char>()));

    const g2::gfx::MaterialMap *materialMap = g2::gfx::GetMaterialMap(materialsBytes.data());
    const g2::gfx::MaterialDef *mat = materialMap->materials()->Get(0);


    std::ifstream imageStream(mat->albedo()->c_str(), std::ios::binary);
    std::vector<char> imageBytes((std::istreambuf_iterator<char>(imageStream)),
                                 (std::istreambuf_iterator<char>()));
    auto image = gfx.addImage(imageBytes);

    std::ifstream imageStreamN(mat->normal()->c_str(), std::ios::binary);
    std::vector<char> imageBytesN((std::istreambuf_iterator<char>(imageStreamN)),
                                  (std::istreambuf_iterator<char>()));

    auto imageN = gfx.addImage(imageBytesN);


    g2::gfx::DrawItem drawItems[]{
            {
                    .mesh = mesh,
                    .image = image,
            },
            {
                    .mesh = mesh,
                    .image = imageN,
            }
    };


    float r = 0;

    while (!app.shouldClose()) {
        app.pollEvents();
        gfx.setFramebufferExtent(app.getWindowSize());


        g2::gfx::Transform transforms[]{
                {
                        .pos = {1, 0, 0},
                        .scale = 1.0f,
                        .orientation = glm::quat(glm::vec3(0, r, 0)),
                },
                {
                        .pos = {-1, 0, 0},
                        .scale = 1.0f,
                        .orientation = glm::quat(),
                }
        };

        gfx.draw(drawItems, transforms);

        r += 0.0001;

    }

}