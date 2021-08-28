//
// Created by felix on 28/08/2021.
//

#ifndef G2_TRANSFORM_H
#define G2_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <g2/ecs/type.h>

namespace g2 {

    struct Transform {
        glm::vec3 pos;
        float scale;
        glm::quat orientation;

        [[nodiscard]] inline glm::mat4 matrix() const {
            //TODO: scale
            auto r = glm::toMat3(orientation);
            glm::mat4 mat;
            mat[0] = glm::vec4(r[0], 0);
            mat[1] = glm::vec4(r[1], 0);
            mat[2] = glm::vec4(r[2], 0);
            mat[3] = glm::vec4(pos, 1);
            return mat;
        }

        [[nodiscard]] inline glm::vec3 apply(const glm::vec3& p) const {
            return (orientation * p) * scale + pos;
        }

        [[nodiscard]] inline Transform inverse() const {
            Transform result{};
            result.pos = glm::inverse(orientation) * ((-pos) / scale);
            result.orientation = glm::inverse(orientation);
            result.scale = 1.0f / scale;
            return result;
        }
    };
    extern ecs::id_t c_transform;


}

#endif //G2_TRANSFORM_H
