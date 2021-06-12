#version 450

#include "transform.glsl"
#include "scene.glsl"
#include "geometry.glsl"

struct DrawData {
    uint baseIndex;
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};

layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout(push_constant) uniform PushConstant {
    uint drawIndex;
};


void main() {
    uint vertexIndex = gl_VertexIndex + drawData[drawIndex].baseVertex;
    Vertex vertex = vertices[vertexIndex];

    Transform transform = transforms[drawIndex];

    vec3 pos = applyTransform(vertex.pos.xyz, transform);

    gl_Position = viewProj * vec4(pos, 1.0);

}
