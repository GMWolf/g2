#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1)  uniform sampler2D textures[];


layout(location = 0) in vec2 uv;

void main() {
    outColor = vec4(texture(textures[0], uv).rgb, 1.0);
    //outColor = vec4(fragColor, 1.0);
}