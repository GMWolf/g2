//
// Created by felix on 13/04/2021.
//

#ifndef G2_GFX_INSTANCE_H
#define G2_GFX_INSTANCE_H
#include <g2/application.h>

#include <glm/glm.hpp>
#include <memory>
#include <span>

#include "command_encoder.h"
#include "viewport.h"

namespace g2::gfx {

void init();

struct InstanceConfig {
  Application *application;
  std::span<const char *> vkExtensions;
};

struct PipelineDef;
struct Pipeline;

class Instance {
  struct Impl;
  std::unique_ptr<Impl> pImpl;

 public:
  explicit Instance(const InstanceConfig &config);
  ~Instance();

  void setFramebufferExtent(glm::ivec2 size);

  const Pipeline* createPipeline(const PipelineDef* pipeline_def);

  [[nodiscard]] bool beginFrame();
  void endFrame();

  CommandEncoder beginRenderpass();
  void endRenderpass(CommandEncoder& command_encoder);
};
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
