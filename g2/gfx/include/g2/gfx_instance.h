//
// Created by felix on 13/04/2021.
//

#ifndef G2_GFX_INSTANCE_H
#define G2_GFX_INSTANCE_H
#include <g2/application.h>

#include <glm/glm.hpp>
#include <memory>
#include <span>

#include "viewport.h"

struct VkPipeline_T;
typedef VkPipeline_T* VkPipeline;

namespace g2::gfx {

void init();

struct InstanceConfig {
  Application *application;
  std::span<const char *> vkExtensions;
};

struct Mesh;

struct PipelineDef;
struct MeshData;

class Instance {
  struct Impl;
  std::unique_ptr<Impl> pImpl;

 public:
  explicit Instance(const InstanceConfig &config);
  ~Instance();

  void setFramebufferExtent(glm::ivec2 size);

  VkPipeline createPipeline(const PipelineDef* pipeline_def);
  const Mesh* addMesh(const MeshData* meshData);


  void draw();

};
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
