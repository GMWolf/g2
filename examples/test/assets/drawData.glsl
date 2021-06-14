
struct DrawData {
    uint baseIndex;
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};