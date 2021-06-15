struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 texcoords;
};

struct PackedVertex {
    uint posxy;
    uint posznormal;
    uint texcoords;
};

layout(set = 0, binding = 0) buffer Vertices {
    PackedVertex vertices[];
};

layout(set = 0, binding = 1) buffer Indices {
    uint indices[];
};


vec2 signNotZero(vec2 v) {
    return vec2((v.x >= 0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

vec3 oct_to_vec3(vec2 e) {
    vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    return normalize(v);
}


Vertex unpackVertex(PackedVertex packed) {
    Vertex vertex;

    vertex.pos.xy = unpackHalf2x16(packed.posxy);
    vertex.pos.z = unpackHalf2x16(packed.posznormal).x;

    vertex.normal = oct_to_vec3(unpackSnorm4x8(packed.posznormal).zw);
    vertex.texcoords = unpackHalf2x16(packed.texcoords);

    return vertex;
}
