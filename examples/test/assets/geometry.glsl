struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 texcoords;
};

layout(set = 0, binding = 0) buffer Vertices {
    uint vertexData[];
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


vec3 loadPosition(uint offset, uint index) {
    //uint a = vertexData[offset + index * 2];
    //uint b = vertexData[offset + index * 2 + 1];
    //return vec4(unpackHalf2x16(a), unpackHalf2x16(b)).xyz;
    return vec3(
        uintBitsToFloat(vertexData[offset + index * 3 + 0]),
        uintBitsToFloat(vertexData[offset + index * 3 + 1]),
        uintBitsToFloat(vertexData[offset + index * 3 + 2])
    );
}

vec3 loadNormal(uint offset, uint index) {
    return oct_to_vec3(unpackSnorm2x16(vertexData[offset + index]));
}

vec3 loadTangent(uint offset, uint index) {
    return vec3(
        uintBitsToFloat(vertexData[offset + index * 3 + 0]),
        uintBitsToFloat(vertexData[offset + index * 3 + 1]),
        uintBitsToFloat(vertexData[offset + index * 3 + 2])
    );
}

vec3 loadBiTangent(uint offset, uint index) {
    return vec3(
        uintBitsToFloat(vertexData[offset + index * 3 + 0]),
        uintBitsToFloat(vertexData[offset + index * 3 + 1]),
        uintBitsToFloat(vertexData[offset + index * 3 + 2])
    );
}

vec2 loadTexcoord(uint offset, uint index) {
    uint a = vertexData[offset + index];
    return unpackHalf2x16(a);
}

//Vertex unpackVertex(PackedVertex packed) {
//    Vertex vertex;
//
//    vertex.pos.xy = unpackHalf2x16(packed.posxy);
//    vertex.pos.z = unpackHalf2x16(packed.posznormal).x;
//
//    vertex.normal = oct_to_vec3(unpackSnorm4x8(packed.posznormal).zw);
//    vertex.texcoords = unpackHalf2x16(packed.texcoords);
//
//    return vertex;
//}
