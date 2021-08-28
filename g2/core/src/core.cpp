//
// Created by felix on 28/08/2021.
//

#include "core.h"

namespace g2 {
    ecs::id_t c_transform;

    void registerCoreComponents(g2::ecs::Registry &registry) {
        c_transform = registry.registerComponent<Transform>();
    }
}
