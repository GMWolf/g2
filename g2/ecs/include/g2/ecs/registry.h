//
// Created by felix on 28/08/2021.
//

#ifndef G2_REGISTRY_H
#define G2_REGISTRY_H

#include "type.h"
#include <vector>

namespace g2::ecs {


    struct Chunk {
        Type type;
        size_t capacity;
        size_t size;
        std::vector<void*> components;
    };

    struct Archetype {
       Type type;
       std::vector<id_t> entities;
       std::vector<Chunk*> chunks;
    };

    struct EntityRecord {
        Chunk* chunk;
        size_t row;
    };

    struct Registry {
        std::vector<Archetype> archetypes;
        std::vector<EntityRecord> records;

        id_t create(const Type& type);
        id_t registerComponent(const Component& component);

        void* get(id_t entity, id_t component, size_t size);

        template<class T>
        T& get(id_t entity, id_t component) {
            return *static_cast<T*>(get(entity, component, sizeof(T)));
        }

        template<class T>
        id_t registerComponent() {
            return registerComponent(Component{
                .size = sizeof(T),
                .align = sizeof(T),
            });
        }

        Registry();
    };

}

#endif //G2_REGISTRY_H
