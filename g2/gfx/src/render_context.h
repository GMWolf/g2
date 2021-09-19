//
// Created by felix on 29/08/2021.
//

#ifndef G2_RENDER_CONTEXT_H
#define G2_RENDER_CONTEXT_H
#include <cstdint>
#include <g2/gfx_instance.h>
#include "mesh.h"
#include "culling.h"
#include "effect.h"

namespace g2::gfx {

    struct DrawData {
        uint32_t baseIndex;
        uint32_t positionOffset;
        uint32_t normalOffset;
        uint32_t texcoordOffset;
        uint32_t tangentOffset;
        uint32_t bitangentOffset;
        uint32_t materialId;
    };

    struct RenderContext {
        VkCommandBuffer cmd{};
        std::span<DrawItem> drawItems;
        std::span<Transform> transforms;
        Transform camera;
        MeshAssetManager* meshManager;
        CameraCullData cameraCullData;
        Transform* transformMap;
        DrawData* drawDataMap;
        VkPipelineLayout pipelineLayout;
        VkBuffer indexBuffer;
        uint32_t maxMaterialId;
        Effect* effect;
        VkPipeline* pipelines;
    };

}

#endif //G2_RENDER_CONTEXT_H
