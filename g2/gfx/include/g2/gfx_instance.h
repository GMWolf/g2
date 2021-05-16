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
#include <glm/gtx/quaternion.hpp>

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
};

struct Transform {
    glm::vec3 pos;
    float scale;
    glm::quat orientation;

    inline glm::mat4 matrix() const {
        //TODO: scale
        auto r = glm::toMat3(orientation);
        glm::mat4 mat;
        mat[0] = glm::vec4(r[0], 0);
        mat[1] = glm::vec4(r[1], 0);
        mat[2] = glm::vec4(r[2], 0);
        mat[3] = glm::vec4(pos, 1);
        return mat;
    }

    inline Transform inverse() const {
        //TODO scale
        Transform result;
        result.pos = glm::inverse(orientation) * -pos;
        result.orientation = glm::inverse(orientation);
        result.scale = 1.0f;
        return result;
    }
};

class Instance {
  struct Impl;
  std::unique_ptr<Impl> pImpl;

 public:

  explicit Instance(const InstanceConfig &config);
  ~Instance();

  std::span<IAssetManager*> getAssetManagers();

  void setFramebufferExtent(glm::ivec2 size);

  void draw(std::span<DrawItem> drawItems, std::span<Transform> transforms, Transform camera);

};
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
