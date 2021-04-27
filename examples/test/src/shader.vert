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

vec3 rotate(vec3 vec, vec4 quat) {
    vec3 t = 2 * cross(quat.xyz, vec);
    return vec + quat.w * t + cross(quat.xyz, t);
}

struct Transform {
    vec3 position;
    float scale;
    vec4 orientation;
};

vec3 applyTransform(vec3 pos, Transform transform) {
    return (rotate(pos, transform.orientation) * transform.scale) + transform.position;
}

layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout( push_constant ) uniform PusConstant {
    uint drawIndex;
};

layout(location = 0) out vec2 uv;

void main() {

    uint vertexIndex = gl_VertexIndex + drawData[drawIndex].baseVertex;
    Vertex vertex = vertices[vertexIndex];

    Transform transform = transforms[drawIndex];

    gl_Position = viewProj * vec4(applyTransform(vertex.pos.xyz, transform), 1.0);
    uv = vertices[gl_VertexIndex].texcoords.xy;
}
