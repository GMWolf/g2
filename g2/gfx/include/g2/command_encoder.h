//
// Created by felix on 17/04/2021.
//

#ifndef G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_
#define G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_

#include "viewport.h"
#include <cstdint>

struct VkPipeline_T;
typedef VkPipeline_T* VkPipeline;

namespace g2::gfx {
    struct Mesh;

struct CommandEncoder {
  uintptr_t cmdbuf;
  void bind_pipeline(VkPipeline pipeline);
  void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
  void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstinstance);
  void draw(const Mesh* mesh);
};

}

#endif  // G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_
