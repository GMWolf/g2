#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

#define UINT32_MAX    (4294967295U)
#define INVALID_IMAGE UINT32_MAX

#include "pbr.glsl"
#include "normals.glsl"
#include "scene.glsl"

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 viewDir;
layout(location = 3) in vec3 worldPos;

layout(set = 0, binding = 1)  uniform sampler2D textures[];
layout(set = 0, binding = 3) uniform sampler2D shadowMap;
struct MaterialData {
    uint albedo;
    uint normal;
    uint metallicRoughness;
    uint occlusion;

    uint emmisive; uint pad0, pad1, pad2;

    vec4 albedoMetallicFactor;
    vec4 emissiveRoughnessFactor;
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

vec4 sampleImage(uint imageIndex, vec2 uv, vec4 def) {
    if (imageIndex == INVALID_IMAGE) {
        return def;
    } else {
        return texture(textures[imageIndex], uv);
    }
}


float shadowIntensity(vec3 pos) {
    vec4 shadowCoord = ((shadowMat) * vec4(pos, 1.0));
    shadowCoord.xy /= shadowCoord.w;
    shadowCoord.xy = (shadowCoord.xy + 1) / 2.0f;
    vec4 shadowFrag =  texture(shadowMap, shadowCoord.xy);

    return int(shadowFrag.x > shadowCoord.z);

    //vec2 moments = shadowFrag.xy;
    //return ChebyshecUpperBound(moments, shadowCoord.z);
}

void main() {

    DrawData d = drawData[drawIndex];

    MaterialData material = materials[d.materialIndex];

    vec4 albedoAlpha = sampleImage(material.albedo, uv, vec4(material.albedoMetallicFactor.rgb, 1.0));

    if(albedoAlpha.a < 0.05) {
        discard;
    }

    PBRFragment pbr;
    pbr.albedo = albedoAlpha.rgb;
    pbr.metalicity = sampleImage(material.metallicRoughness, uv, vec4(material.albedoMetallicFactor.a)).b;
    pbr.roughness = sampleImage(material.metallicRoughness, uv, vec4(material.emissiveRoughnessFactor.a)).g;
    pbr.emmisivity = sampleImage(material.emmisive, uv, vec4(material.emissiveRoughnessFactor.rgb, 1)).rgb;

    pbr.normal = normalize(normal);
    if (material.normal != INVALID_IMAGE) {
        pbr.normal = perturb_normal(pbr.normal, viewDir, uv, texture(textures[material.normal], uv).rgb);
    }

    LightFragment light;
    light.lightDirection = -normalize(vec3(-0.75, -1, 0.35));



    light.radiance = vec3(shadowIntensity(worldPos + (normal * 0.01)));//vec3(2.0);

    vec3 col = pbrColor(pbr, light, normalize(viewDir));
    vec3 ambient =  pbr.albedo * vec3(0.02) * sampleImage(material.occlusion, uv, vec4(1)).r;


    outColor = vec4(col + ambient, 1.0);
}