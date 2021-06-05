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