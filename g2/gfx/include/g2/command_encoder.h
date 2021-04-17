//
// Created by felix on 17/04/2021.
//

#ifndef G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_
#define G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_

#include "viewport.h"
#include <cstdint>

namespace g2::gfx {

struct Pipeline;

struct CommandEncoder {
  uintptr_t cmdbuf;
  void bind_pipeline(const Pipeline* pipeline);
  void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
};

}

#endif  // G2_G2_GFX_INCLUDE_G2_COMMAND_ENCODER_H_
