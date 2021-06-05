layout(set = 1, binding = 0) uniform Scene {
    mat4 viewProj;
    vec3 viewPos; float pad;
    mat4 shadowMat;
};