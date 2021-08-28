//
// Created by felix on 28/08/2021.
//

#ifndef G2_SCENE_COMPILER_H
#define G2_SCENE_COMPILER_H

#include <cgltf.h>
#include <vector>
#include <cstdint>


std::vector<uint8_t> compileScene(const cgltf_node* nodes, size_t nodeCount);

#endif //G2_SCENE_COMPILER_H
