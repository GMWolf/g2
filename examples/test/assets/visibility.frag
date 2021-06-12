#version 450

layout(location = 0) out uint outVB;


struct DrawData {
    uint baseIndex;
    uint baseVertex;
    uint materialIndex;
};


layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};

layout( push_constant ) uniform PushConstant {
    uint drawIndex;
};

uint computeVB(uint drawId, uint primId) {
    return ( drawId << 23 ) & 0x7F800000u | (primId & 0x007FFFFFu);
}


void main() {
    outVB = computeVB( drawIndex,  gl_PrimitiveID );
}
