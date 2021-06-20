//
// Created by felix on 20/06/2021.
//

#ifndef G2_CULLING_H
#define G2_CULLING_H

#include <g2/gfx_instance.h>
#include "mesh.h"

namespace g2::gfx {

    struct CameraCullData {
        Transform cameraTransform;
        glm::vec2 sphereFactor;
        float tang;
        float aspect;
        float near;
        float far;
    };


    inline CameraCullData buildCameraCullData(const Transform &cameraTransform, float fov, float aspect, float near, float far) {
        CameraCullData data{};
        data.cameraTransform = cameraTransform;
        data.tang = tanf(fov * 0.5f);
        data.aspect = aspect;
        float fovx = atanf(data.tang * data.aspect);
        data.sphereFactor.x = 1.0f / cosf(fovx);
        data.sphereFactor.y = 1.0f / cosf(fov * 0.5f);
        data.near = near;
        data.far = far;
        return data;
    }

    inline bool meshletInView(const CameraCullData &cc, const Meshlet& meshlet, const Transform &transform) {

        // rmt_ScopedCPUSample(MeshletInView, RMTSF_Aggregate);

       glm::vec3 pos = transform.apply(meshlet.center);
       float radius = meshlet.radius * transform.scale;
       pos = cc.cameraTransform.inverse().apply(pos);

       // Frustum culling
       if (-pos.z > cc.far + radius || -pos.z < cc.near - radius) {
           return false;
       }

       glm::vec2 d = cc.sphereFactor * radius;

       float az = -pos.z * cc.tang;
       if (pos.y > az + d.y || pos.y < -az - d.y) {
           return false;
       }

       az *= cc.aspect;
       if (pos.x > az + d.x || pos.x < -az - d.x) {
           return false;
       }

        // cone backface culling
        glm::vec3 coneApex = transform.apply(meshlet.coneApex);
        glm::vec3 coneAxis = transform.orientation * meshlet.coneAxis;
        if (glm::dot(normalize(coneApex - cc.cameraTransform.pos), coneAxis) >= meshlet.coneCutoff) {
            return false;
        }

        return true;
    }
}
#endif //G2_CULLING_H
