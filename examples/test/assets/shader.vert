#version 450

#include "transform.glsl"
#include "scene.glsl"
#include "geometry.glsl"


struct DrawData {
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};


layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout( push_constant ) uniform PusConstant {
    uint drawIndex;
};

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 viewDir;

void main() {
    uint vertexIndex = gl_VertexIndex + drawData[drawIndex].baseVertex;
    Vertex vertex = vertices[vertexIndex];

    Transform transform = transforms[drawIndex];

    vec3 pos = applyTransform(vertex.pos.xyz, transform);

    gl_Position = viewProj * vec4(pos, 1.0);
    uv = vertex.texcoords.xy;
    normal = rotate(vertex.normal.rgb, transform.orientation);
    viewDir = viewPos - pos;
}
