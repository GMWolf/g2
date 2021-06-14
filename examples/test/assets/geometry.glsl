struct Vertex {
    vec4 pos;
    vec4 normal;
    vec4 texcoords;
};

layout(set = 0, binding = 0) buffer Vertices {
    Vertex vertices[];
};

layout(set = 0, binding = 1) buffer Indices {
    uint indices[];
};
