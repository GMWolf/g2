//
// Created by felix on 28/08/2021.
//

#include <g2/ecs/registry.h>
#include <iostream>

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

    auto e = registry.create({pos, vel});
    *registry.get<Position>(e, pos) = {
            .x = 0,
            .y = 0,
    };
    *registry.get<Velocity>(e, vel) = {
            .x = 1,
            .y = 2,
    };


    for(int i = 0; i < 10; i++)
    {
        Position& position = *registry.get<Position>(e, pos);
        Velocity& velocity = *registry.get<Velocity>(e, vel);

        position.x += velocity.x;
        position.y += velocity.y;

        std::cout << "pos: " << position.x << " " << position.y << std::endl;
    }


    return 0;
}