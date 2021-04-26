#version 450

struct Vertex {
    vec4 pos;
    vec4 normal;
    vec4 texcoords;
};

layout(set = 0, binding = 0) buffer Vertices {
    Vertex vertices[];
};

layout(set = 1, binding = 0) uniform Scene {
    mat4 viewProj;
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

layout(location = 0) out vec2 uv;

void main() {

    uint vertexIndex = gl_VertexIndex + drawData[drawIndex].baseVertex;

    gl_Position = viewProj * vec4(vertices[vertexIndex].pos.xyz, 1.0);
    uv = vertices[gl_VertexIndex].texcoords.xy;
}
