#version 450

#include "drawData.glsl"

layout(set = 0, binding = 5) uniform usampler2D visbuffer;


void main() {
    uint v = texelFetch(visbuffer, ivec2( gl_FragCoord.xy), 0).r;

    if (v == 0)
    {
        gl_FragDepth = 1000.0f;
    }
    else
    {
        v -= 1;

        uint drawIndex = v >> 7;
        uint matId = drawData[drawIndex].materialIndex;

        gl_FragDepth = matId / 1000.0f;
    }
}
