//
// Created by felix on 28/08/2021.
//

#include "scene_compiler.h"
#include <g2/scene/scene_generated.h>

#include <filesystem>

namespace fb = flatbuffers;
namespace fs = std::filesystem;

std::vector<uint8_t> compileScene(const cgltf_node *nodes, size_t nodeCount) {

    fb::FlatBufferBuilder fbb(1024);

    std::vector<fb::Offset<g2::Node>> fbNodes;

    for(int i = 0; i < nodeCount; i++) {

        const cgltf_node& node = nodes[i];

        if (node.mesh == nullptr) {
            continue;
        }

        g2::NodeTransform transform(node.translation, node.scale[0], node.rotation);

        std::string meshName = std::string(node.mesh->name) + ".g2mesh";

        fbNodes.push_back(g2::CreateNodeDirect(fbb, &transform, meshName.c_str()));
    }

    auto scene = g2::CreateSceneDirect(fbb, &fbNodes);
    g2::FinishSceneBuffer(fbb, scene);

    return std::vector<uint8_t>(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
}
