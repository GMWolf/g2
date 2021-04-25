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

layout(location = 0) out vec2 uv;

void main() {
    gl_Position = viewProj * vec4(vertices[gl_VertexIndex].pos.xyz, 1.0);
    uv = vertices[gl_VertexIndex].texcoords.xy;
}
