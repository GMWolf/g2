//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <g2/gfx_instance.h>


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

    while(!app.shouldClose()) {
      app.pollEvents();
      gfx.setFramebufferExtent(app.getWindowSize());
      gfx.drawFrame();
    }

}