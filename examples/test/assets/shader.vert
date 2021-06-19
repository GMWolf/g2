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

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 viewDir;
layout(location = 3) out vec3 worldPos;
layout(location = 4) out vec4 shadowCoord;

void main() {
    DrawData d = drawData[drawIndex];

    vec3 inPos = loadPosition(d.positionOffset, gl_VertexIndex);
    vec3 inNormal = loadNormal(d.normalOffset, gl_VertexIndex);
    vec2 inTexcoord = loadTexcoord(d.texcoordOffset, gl_VertexIndex);

    Transform transform = transforms[drawIndex];

    vec3 pos = applyTransform(inPos, transform);
    worldPos = pos;

    gl_Position = viewProj * vec4(pos, 1.0);
    uv = inTexcoord;
    normal = rotate(inNormal, transform.orientation);
    viewDir = viewPos - pos;

    shadowCoord = (shadowMat * vec4(pos + normal * 0.1, 1.0));
    shadowCoord.xy = (shadowCoord.xy + 1) / 2.0f;
}
