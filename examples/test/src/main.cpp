//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <g2/gfx_instance.h>
#include <fstream>
#include <vector>
#include <g2/gfx/pipeline_generated.h>
#include <g2/gfx/mesh_generated.h>

struct Vertex {
  float pos[4];
  float col[4];
};


int main()
{
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

  std::ifstream pipelineDefInput("pipeline.json.bin", std::ios::binary);
  std::vector<char> pipelineBytes((std::istreambuf_iterator<char>(pipelineDefInput)),
                                (std::istreambuf_iterator<char>()));

  VkPipeline pipeline = gfx.createPipeline(g2::gfx::GetPipelineDef(pipelineBytes.data()));


  std::ifstream meshStream("DamagedHelmet/DamagedHelmet.gltf.mesh_helmet_LP_13930damagedHelmet.bin", std::ios::binary);
  std::vector<char> meshBytes((std::istreambuf_iterator<char>(meshStream)),
                              (std::istreambuf_iterator<char>()));

  const g2::gfx::MeshData* meshData = g2::gfx::GetMeshData(meshBytes.data());

  auto mesh = gfx.addMesh(meshData);

  std::ifstream imageStream("DamagedHelmet/Default_albedo.jpg.ktx2", std::ios::binary);
  std::vector<char> imageBytes((std::istreambuf_iterator<char>(imageStream)),
                               (std::istreambuf_iterator<char>()));
  auto image = gfx.addImage(imageBytes);

    std::ifstream imageStreamN("DamagedHelmet/Default_normal.jpg.ktx2", std::ios::binary);
    std::vector<char> imageBytesN((std::istreambuf_iterator<char>(imageStreamN)),
                                 (std::istreambuf_iterator<char>()));

  auto imageN = gfx.addImage(imageBytesN);

  g2::gfx::DrawItem drawItems[] {
          {
              .mesh = mesh,
              .image = image,
          }
  };

  while(!app.shouldClose()) {
    app.pollEvents();
    gfx.setFramebufferExtent(app.getWindowSize());

    gfx.draw(drawItems);

  }

}