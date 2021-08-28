//
// Created by felix on 28/08/2021.
//

#include "registry.h"

namespace g2::ecs {

    static Archetype* findArchetype(Registry& registry, const Type& type) {
        for(Archetype& archetype : registry.archetypes) {
            if (archetype.type == type) {
                return &archetype;
            }
        }
        return nullptr;
    }

    static Archetype& getArchetype(Registry& registry, const Type& type) {
        Archetype* archetype = findArchetype(registry, type);
        if (!archetype) {
            archetype= &registry.archetypes.emplace_back(Archetype{
                .type = type
            });
        }
        return *archetype;
    }

    static int typeGetComponentIndex(const Type& type, id_t component) {
        for(int index = 0; index < type.components.size(); index++) {
            if (type.components[index] == component) {
                return index;
            }
        }
        return -1;
    }

    static void* getComponent(Registry& registry, id_t entity, id_t component, size_t componentSize) {
        auto record = registry.records[entity];
        auto index = typeGetComponentIndex(record.chunk->type, component);
        if (index < 0) {
            return nullptr;
        }

        return static_cast<char*>(record.chunk->components[index]) + (record.row * componentSize);
    }

    static Chunk* createChunk(Registry& registry, const Type& type, size_t capacity) {
        auto* chunk = new Chunk {
            .type = type,
            .capacity = capacity,
            .size = 0,
            .components = std::vector<void*>(type.components.size()),
        };

        for(int i = 0; i < type.components.size(); i++) {
            auto* component = static_cast<Component*>(getComponent(registry, 0, 0, sizeof(Component)));
            size_t componentSize = component->size;
            chunk->components[i] = malloc(capacity * componentSize);
        }

        return chunk;
    }

    static Chunk* getFreeChunk(Registry& registry, Archetype& archetype, const Type& type) {
        for(auto chunk : archetype.chunks) {
            if (chunk->size < chunk->capacity) {
                return chunk;
            }
        }

        size_t capacity = 1024;
        return archetype.chunks.emplace_back(createChunk(registry, type, capacity));
    }


    id_t Registry::create(const Type &type) {
        Archetype& archetype = getArchetype(*this, type);
        Chunk* chunk = getFreeChunk(*this, archetype, type);

        records.push_back(EntityRecord{
            .chunk = chunk,
            .row = chunk->size
        });

        chunk->size += 1;

        return records.size() - 1;
    }

    static void bootstrap(Registry& registry) {

        Type componentType {0};

        auto archetype = registry.archetypes.emplace_back(Archetype{
            .type = componentType,
        });

        size_t capacity = 1024;

        auto chunk = archetype.chunks.emplace_back( new Chunk{
            .type = componentType,
            .capacity = capacity,
            .size = 1,
            .components {
                malloc(capacity * sizeof(Component))
            },
        });

        auto* component = new(chunk->components[0]) Component;
        component->size = sizeof(Component);
        component->align = alignof(Component);

        registry.records.push_back(EntityRecord{
            .chunk = chunk,
            .row = 0
        });
    }

    Registry::Registry() {
        bootstrap(*this);
    }

    id_t Registry::registerComponent(const Component& component) {
        id_t id = create({0});
        void* a = getComponent(*this, id, 0, sizeof(Component));
        new(a) Component(component);
        return id;
    }

    void *Registry::get(id_t entity, id_t component, size_t size) {
        return getComponent(*this, entity, component, size);
    }


}