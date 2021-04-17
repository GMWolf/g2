//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <g2/gfx_instance.h>
#include <fstream>
#include <vector>
#include <g2/gfx/pipeline_generated.h>

int main()
{
  g2::ApplicationConfiguration appConfig{
      .width = 1280,
      .height = 720,
      .title = "Application"
  };

  auto app = g2::Application(appConfig);

  g2::gfx::InstanceConfig gfxConfig{
      .application = &app,
      .vkExtensions = g2::getVkExtensions(),
  };

  auto gfx = g2::gfx::Instance(gfxConfig);

  std::ifstream pipelineDefInput("pipeline.bin", std::ios::binary);
  std::vector<char> vertexBytes((std::istreambuf_iterator<char>(pipelineDefInput)),
                                (std::istreambuf_iterator<char>()));

  const g2::gfx::Pipeline* pipeline = gfx.createPipeline(g2::gfx::GetPipelineDef(vertexBytes.data()));

  while(!app.shouldClose()) {
    app.pollEvents();
    gfx.setFramebufferExtent(app.getWindowSize());

    if (gfx.beginFrame()) {
      auto encoder = gfx.beginRenderpass();
      encoder.bind_pipeline(pipeline);
      encoder.draw(3, 1, 0, 0);
      gfx.endRenderpass(encoder);
      gfx.endFrame();
    }

  }

}