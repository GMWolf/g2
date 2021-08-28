//
// Created by felix on 28/08/2021.
//

#include <g2/ecs/registry.h>
#include <iostream>
#include <g2/ecs/query.h>
#include <g2/ecs/view.h>

using namespace g2::ecs;

struct Position {
    int x, y;
};

struct Velocity {
    int x, y;
};

int main() {

    Registry registry;

    auto pos = registry.registerComponent<Position>();
    auto vel = registry.registerComponent<Velocity>();

    for(int i = 0; i < 20; i++) {
        auto e = registry.create({pos, vel});
        registry.get<Position>(e, pos) = {
                .x = 0,
                .y = 0,
                };
        registry.get<Velocity>(e, vel) = {
                .x = 1,
                .y = 2,
                };
    }

    Query q;
    q.components = {pos, vel};

    for(int i = 0; i < 10; i++) {
        for(Chunk& chunk : query(registry, q)) {
            auto view = ChunkView<Position, Velocity>(chunk, pos, vel);
            for(auto[position, velocity] : view) {
                position.x += velocity.x;
                position.y += velocity.y;

                std::cout << "pos: " << position.x << " " << position.y << std::endl;
            }
        }
    }

    return 0;
}