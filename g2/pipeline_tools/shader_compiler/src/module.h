//
// Created by felix on 24/06/2021.
//

#ifndef G2_MODULE_H
#define G2_MODULE_H

#include <vector>
#include <string>
#include <filesystem>
#include <g2/gfx/vk_generated.h>

std::vector<uint32_t> compileModule(const char* text, const char* filename, g2::gfx::ShaderStage stage, std::vector<std::filesystem::path>& depsVec);

#endif //G2_MODULE_H
