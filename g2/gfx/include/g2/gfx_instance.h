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

#include <g2/core/Transform.h>

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

    struct RenderContext;
    void draw(RenderContext* ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
