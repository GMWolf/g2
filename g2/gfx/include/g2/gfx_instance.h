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
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <g2/assets/asset_registry.h>

struct VkPipeline_T;
typedef VkPipeline_T* VkPipeline;

namespace g2::gfx {

void init();

struct InstanceConfig {
  Application *application;
  std::span<const char *> vkExtensions;
};

struct PipelineDef;
struct MeshData;

struct DrawItem {
    uint32_t mesh;
    uint32_t image;
};

struct Transform {
    glm::vec3 pos;
    float scale;
    glm::quat orientation;
};

class Instance {
  struct Impl;
  std::unique_ptr<Impl> pImpl;

 public:

  explicit Instance(const InstanceConfig &config);
  ~Instance();

  IAssetManager* getImageManager();
  IAssetManager* getMeshManager();

  void setFramebufferExtent(glm::ivec2 size);

  VkPipeline createPipeline(const PipelineDef* pipeline_def);

  void draw(std::span<DrawItem> drawItems, std::span<Transform> transforms);

};
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
