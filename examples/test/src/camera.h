//
// Created by felix on 28/08/2021.
//

#ifndef G2_CAMERA_H
#define G2_CAMERA_H

#include <g2/ecs/registry.h>
#include <g2/input.h>
#include <g2/ecs/query.h>
#include <g2/ecs/view.h>

struct FPSController {
    float movSpd;
};
id_t c_fpsController;


void updateCameras(g2::ecs::Registry& ecs, g2::InputState input, float dt) {

    g2::ecs::Query q;
    q.components = {g2::c_transform, g2::c_camera};

    for(auto chunk : g2::ecs::query(ecs, q)) {
        for(auto[transform] : g2::ecs::ChunkView<g2::Transform>(chunk, g2::c_transform)) {

            float movSpd = 8;

            if (input.keyDown(g2::KEYS::W)) {
                transform.pos += transform.orientation * glm::vec3(0,0,-1) * dt * movSpd;
            }
            if (input.keyDown(g2::KEYS::S)) {
                transform.pos += transform.orientation * glm::vec3(0,0,1) * dt * movSpd;
            }
            if (input.keyDown(g2::KEYS::A)) {
                transform.pos += transform.orientation * glm::vec3(-1, 0, 0) * dt * movSpd;
            }
            if (input.keyDown(g2::KEYS::D)) {
                transform.pos += transform.orientation * glm::vec3(1, 0, 0) * dt * movSpd;
            }
            if (input.keyDown(g2::KEYS::E)) {
                transform.orientation *= glm::quat(glm::vec3(0, -1, 0) * dt);
            }
            if (input.keyDown(g2::KEYS::Q)) {
                transform.orientation *= glm::quat(glm::vec3(0, 1, 0) * dt);
            }
            if (input.keyDown(g2::KEYS::SPACE)) {
                transform.pos += transform.orientation * glm::vec3(0, 1, 0) * dt * movSpd;
            }
            if (input.keyDown(g2::KEYS::LEFT_CONTROL)) {
                transform.pos += transform.orientation * glm::vec3(0, -1, 0) * dt * movSpd;
            }

        }
    }

}

#endif //G2_CAMERA_H
