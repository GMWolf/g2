layout(set = 1, binding = 0) uniform Scene {
    mat4 viewProj;
    mat4 view;
    mat4 projection;
    vec3 viewPos; float pad;
    mat4 shadowMat;
    vec2 shadowmapscale;
};