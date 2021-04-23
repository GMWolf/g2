//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <g2/gfx_instance.h>
#include <fstream>
#include <vector>
#include <g2/gfx/pipeline_generated.h>

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

  std::ifstream pipelineDefInput2("pipeline2.json.bin", std::ios::binary);
  std::vector<char> pipelineBytes2((std::istreambuf_iterator<char>(pipelineDefInput2)),
                                (std::istreambuf_iterator<char>()));

  VkPipeline pipeline2 = gfx.createPipeline(g2::gfx::GetPipelineDef(pipelineBytes2.data()));

  while(!app.shouldClose()) {
    app.pollEvents();
    gfx.setFramebufferExtent(app.getWindowSize());

    if (gfx.beginFrame()) {
      auto encoder = gfx.beginRenderpass();
      encoder.bind_pipeline(pipeline);
      encoder.drawIndexed(3, 1, 0, 0, 0);
      encoder.bind_pipeline(pipeline2);
      encoder.drawIndexed(3, 1, 0, 0, 0);
      gfx.endRenderpass(encoder);
      gfx.endFrame();
    }

  }

}