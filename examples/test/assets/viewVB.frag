#version 450

#include "drawData.glsl"

layout(set = 0, binding = 5) uniform usampler2D visbuffer;

layout(location = 0) in vec2 screenPos;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float random (in vec2 _st) {
    return fract(sin(dot(_st.xy,
    vec2(12.9898,78.233)))*
    43758.5453123);
}


layout(location = 0) out vec4 outColor;

void main() {

    vec2 uv = 0.5 * (screenPos + 0.5);

    //vec2 uv = vec2(screenPos.x, 1 - screenPos.y);

    uint v = texelFetch(visbuffer, ivec2( gl_FragCoord.xy), 0).r;

    uint drawId = v >> 7;
    uint triId = v & 0x7Fu;

    uint material = drawData[drawId].materialIndex;

    vec3 c = hsv2rgb(vec3(random(vec2(drawId, 1.0)), 0.5 + 0.5 * random(vec2(triId, 1.0)), 1.0));

    outColor = vec4(c, 1.0);
}
