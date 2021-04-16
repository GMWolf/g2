//
// Created by felix on 15/04/2021.
//

#ifndef G2_SHADER_H
#define G2_SHADER_H

#include <optional>
#include <span>

#include "vk.h"

namespace g2::gfx {

vk::ShaderModule createShaderModule(vk::Device device, std::span<char> code);

}

#endif  // G2_SHADER_H
