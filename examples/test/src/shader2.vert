#version 450

struct Vertex {
    vec4 pos;
    vec4 color;
    vec4 uv;
};

layout(set = 0, binding = 0) buffer Vertices {
  Vertex vertices[];
};

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex].pos.xy + vec2(0, 0.5), 0.0, 1.0);
    fragColor = vertices[gl_VertexIndex].color.rgb;
    uv = vertices[gl_VertexIndex].uv.xy;
}
