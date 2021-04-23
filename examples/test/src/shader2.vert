#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 1.0)
);

struct Vertex {
    vec4 pos;
    vec4 color;
};

layout(set = 0, binding = 0) buffer Vertices {
  Vertex vertices[];
};

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex].pos.xy + vec2(0, 0.5), 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}