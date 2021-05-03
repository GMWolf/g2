//
// Created by felix on 03/05/2021.
//

#ifndef G2_MATERIAL_COMPILER_H
#define G2_MATERIAL_COMPILER_H

#include <cstdint>
#include <vector>
#include <cgltf.h>

std::vector<uint8_t> compileMaterial(const cgltf_material* material);



#endif //G2_MATERIAL_COMPILER_H
