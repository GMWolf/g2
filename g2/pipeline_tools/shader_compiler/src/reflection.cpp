//
// Created by felix on 27/06/2021.
//

#include "reflection.h"

#include <spirv_cross.hpp>

void reflect(const std::vector<uint32_t> &data) {
    spirv_cross::Compiler compiler(data);


    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    for(auto& resource : resources.sampled_images) {

    }

}
