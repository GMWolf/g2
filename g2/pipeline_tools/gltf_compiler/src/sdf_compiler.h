//
// Created by felix on 26/09/2021.
//

#ifndef G2_SDF_COMPILER_H
#define G2_SDF_COMPILER_H

#include <cgltf.h>
#include <glm/glm.hpp>

std::vector<float> genSDF(const cgltf_mesh *mesh, int width, int height, int depth, float dx, glm::vec3 origin, int exact_band);

#endif //G2_SDF_COMPILER_H
