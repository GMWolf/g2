#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 uv;

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

    outColor = vec4(texture(textures[material.metallicRoughness], uv).rgb, 1.0);
}