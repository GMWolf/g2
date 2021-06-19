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
    DrawData d = drawData[drawIndex];

    vec3 inPos = loadPosition(d.positionOffset, gl_VertexIndex);

    Transform transform = transforms[drawIndex];
    vec3 pos = applyTransform(inPos, transform);

    gl_Position = shadowMat * vec4(pos, 1.0);
}
