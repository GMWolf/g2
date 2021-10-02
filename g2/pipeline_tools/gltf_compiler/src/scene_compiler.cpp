//
// Created by felix on 28/08/2021.
//

#include "scene_compiler.h"
#include <g2/scene/scene_generated.h>

#include <filesystem>
#include "iostream"
#include <span>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fb = flatbuffers;
namespace fs = std::filesystem;

std::vector<uint8_t> compileScene(const cgltf_node *nodes, size_t nodeCount) {

    fb::FlatBufferBuilder fbb(1024);

    std::vector<fb::Offset<g2::Node>> fbNodes;

    for(int i = 0; i < nodeCount; i++) {

        const cgltf_node& node = nodes[i];

        glm::quat rotation (node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);

        for(auto& child : std::span(node.children, node.children_count)) {
            if (std::strcmp(child->name, "Camera_Orientation") == 0) {
                if (strcmp(node.name, "Camera") == 0) {
                    glm::quat c(child->rotation[3], child->rotation[0], child->rotation[1], child->rotation[2]);
                    rotation = rotation * c;
                    std::cout << "Applied child rotation" << std::endl;
                }
            }
        }

        const char* cMeshName = nullptr;
        std::string meshName;

        if (node.mesh) {
            meshName = std::string(node.mesh->name) + ".g2mesh";
            cMeshName = meshName.c_str();
        }

        if (node.name == nullptr)
        {
            std::cerr << "no name for node " << i << ".\n";
        }

        float r[4];
        r[0] = rotation.x;
        r[1] = rotation.y;
        r[2] = rotation.z;
        r[3] = rotation.w;

        g2::NodeTransform transform(node.translation, node.scale[0], r);

        fbNodes.push_back(g2::CreateNodeDirect(fbb, &transform, cMeshName, node.name));
    }

    auto scene = g2::CreateSceneDirect(fbb, &fbNodes);
    g2::FinishSceneBuffer(fbb, scene);

    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}
