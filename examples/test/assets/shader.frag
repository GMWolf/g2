#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "pbr.glsl"
#include "normals.glsl"

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 viewDir;

layout(set = 0, binding = 1)  uniform sampler2D textures[];

struct MaterialData {
    uint albedo;
    uint normal;
    uint metallicRoughness;
    uint occlusion;
    uint emmisive;
};

layout(set = 0, binding = 2) buffer MaterialDataBuffer {
    MaterialData materials[];
};

struct DrawData {
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};


layout( push_constant ) uniform PusConstant {
    uint drawIndex;
};


layout(location = 0) out vec4 outColor;

void main() {

    DrawData d = drawData[drawIndex];

    MaterialData material = materials[d.materialIndex];

    PBRFragment pbr;
    pbr.albedo = texture(textures[material.albedo], uv).rgb;
    pbr.metalicity = texture(textures[material.metallicRoughness], uv).b;
    pbr.roughness = texture(textures[material.metallicRoughness], uv).g;
    pbr.emmisivity = texture(textures[material.emmisive], uv).rgb;
    pbr.normal = perturb_normal(normalize(normal), viewDir, uv, texture(textures[material.normal], uv).rgb);

    LightFragment light;
    light.lightDirection = -normalize(vec3(0.75, -1, 0));
    light.radiance = vec3(1.0f);

    vec3 col = pbrColor(pbr, light, normalize(viewDir));


    outColor = vec4(col, 1.0);
}