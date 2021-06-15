#version 450

#include "transform.glsl"
#include "scene.glsl"
#include "geometry.glsl"
#include "drawData.glsl"


layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout( push_constant ) uniform PusConstant {
    uint drawIndex;
};

void main() {
    uint vertexIndex = gl_VertexIndex + drawData[drawIndex].baseVertex;
    Vertex vertex = unpackVertex(vertices[vertexIndex]);

    Transform transform = transforms[drawIndex];
    vec3 pos = applyTransform(vertex.pos.xyz, transform);

    gl_Position = shadowMat * vec4(pos, 1.0);
}
