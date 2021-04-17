//
// Created by felix on 17/04/2021.
//

#ifndef G2_G2_GFX_INCLUDE_G2_VIEWPORT_H_
#define G2_G2_GFX_INCLUDE_G2_VIEWPORT_H_
#include <glm/vec2.hpp>

namespace g2::gfx {

struct Viewport {
  glm::vec2 pos;
  glm::vec2 extent;
  glm::vec2 minMaxDepth;
};


}

#endif  // G2_G2_GFX_INCLUDE_G2_VIEWPORT_H_
