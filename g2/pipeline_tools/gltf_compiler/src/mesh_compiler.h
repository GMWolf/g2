//
// Created by felix on 03/05/2021.
//

#ifndef G2_MESH_COMPILER_H
#define G2_MESH_COMPILER_H

#include <cgltf.h>
#include <vector>
#include <cstdint>

std::vector<uint8_t> compileMesh(const cgltf_mesh* mesh);


#endif //G2_MESH_COMPILER_H
