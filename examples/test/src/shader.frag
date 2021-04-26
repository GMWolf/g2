#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 uv;

layout(set = 0, binding = 1)  uniform sampler2D textures[];

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

    outColor = vec4(texture(textures[d.materialIndex], uv).rgb, 1.0);
}