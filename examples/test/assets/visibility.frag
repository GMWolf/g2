#version 450
#extension GL_EXT_nonuniform_qualifier : require
#include "drawData.glsl"

layout(location = 0) out uint outVB;
layout(set = 0, binding = 2)  uniform sampler2D textures[];


#define UINT32_MAX    (4294967295U)
#define INVALID_IMAGE UINT32_MAX

vec4 sampleImage(uint imageIndex, vec2 uv, vec4 def) {
    if (imageIndex == INVALID_IMAGE) {
        return def;
    } else {
        return texture(textures[imageIndex], uv);
    }
}

struct MaterialData {
    uint albedo;
    uint normal;
    uint metallicRoughness;
    uint occlusion;

    uint emmisive; uint pad0, pad1, pad2;

    vec4 albedoMetallicFactor;
    vec4 emissiveRoughnessFactor;
};

layout(set = 0, binding = 3) buffer MaterialDataBuffer {
    MaterialData materials[];
};


layout( push_constant ) uniform PushConstant {
    uint drawIndex;
};

uint computeVB(uint drawId, uint primId) {
    return (( drawId << 7u )  | (primId & 0x7Fu)) + 1;
}

layout(location = 0) in vec2 uv;

void main() {

    uint materialId = drawData[drawIndex].materialIndex;
    MaterialData material = materials[materialId];

    vec4 albedoAlpha = sampleImage(material.albedo, uv, vec4(material.albedoMetallicFactor.rgb, 1.0));
    if (albedoAlpha.a <= 0.5) {
        discard;
    }


    outVB = computeVB( drawIndex,  gl_PrimitiveID );
}
