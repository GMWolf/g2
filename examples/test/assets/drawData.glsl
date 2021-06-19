
struct DrawData {
    uint baseIndex;
    uint positionOffset;
    uint normalOffset;
    uint texcoordOffset;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};