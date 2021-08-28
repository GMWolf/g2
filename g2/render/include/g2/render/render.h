//
// Created by felix on 28/08/2021.
//

#ifndef G2_RENDER_H
#define G2_RENDER_H

#include <g2/ecs/type.h>
#include <g2/ecs/registry.h>
#include <g2/gfx_instance.h>

namespace g2 {

    struct MeshRender {
        uint32_t meshIndex;
    };
    extern ecs::id_t c_meshRender;

    struct Camera {
    };
    extern ecs::id_t c_camera;

    void registerRenderComponents(ecs::Registry& registry);

    void render(g2::gfx::Instance& instance, ecs::Registry& ecs);

}

#endif //G2_RENDER_H
