#version 430


layout(location = 0) out vec2 screenPos;

layout( push_constant ) uniform PusConstant {
    uint matId;
};

void main() {
    gl_Position.x = (gl_VertexIndex == 2 ? 3.0 : -1.0);
    gl_Position.y = (gl_VertexIndex == 0 ? -3.0 : 1.0);
    gl_Position.z = matId;
    gl_Position.w = vec2(1.0);
    screenPos = gl_Position.xy;
}
