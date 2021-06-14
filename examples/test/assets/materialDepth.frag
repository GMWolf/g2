#version 450

struct DrawData {
    uint baseIndex;
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};

layout(set = 0, binding = 5) uniform usampler2D visbuffer;


void main() {
    uint v = texelFetch(visbuffer, ivec2( gl_FragCoord.xy), 0).r;

    uint drawIndex = v >> 23;
    uint matId = drawData[drawIndex].materialIndex;

    gl_FragDepth = matId / 1000.0f;
}
