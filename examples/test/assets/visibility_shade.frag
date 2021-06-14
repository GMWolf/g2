#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

#define UINT32_MAX    (4294967295U)
#define INVALID_IMAGE UINT32_MAX

#include "pbr.glsl"
#include "normals.glsl"
#include "scene.glsl"
#include "geometry.glsl"
#include "transform.glsl"

layout(location = 0) in vec2 screenPos;

layout(set = 0, binding = 2)  uniform sampler2D textures[];
layout(set = 0, binding = 4) uniform sampler2DShadow shadowMap;
layout(set = 0, binding = 5) uniform usampler2D visbuffer;

struct MaterialData {
    uint albedo;
    uint normal;
    uint metallicRoughness;
    uint occlusion;

    uint emmisive; uint pad0, pad1, pad2;

    vec4 albedoMetallicFactor;
    vec4 emissiveRoughnessFactor;
};

layout(set = 0, binding = 3) buffer MaterialDataBuffer {
    MaterialData materials[];
};

struct DrawData {
    uint baseIndex;
    uint baseVertex;
    uint materialIndex;
};

layout(set = 1, binding = 1) buffer DrawDataBlock {
    DrawData drawData[];
};

layout( push_constant ) uniform PusConstant {
    uint matId;
};

layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout(early_fragment_tests) in;

layout(location = 0) out vec4 outColor;

vec4 sampleImage(uint imageIndex, vec2 uv, vec4 def) {
    if (imageIndex == INVALID_IMAGE) {
        return def;
    } else {
        return texture(textures[imageIndex], uv);
    }
}

float shadow_offset_lookup(sampler2DShadow map, vec4 loc, vec2 offset)
{
    return textureProj(map, vec4(loc.xy + offset * shadowmapscale * loc.w, loc.z, loc.w));
}

float shadowIntensity(vec4 shadowCoord) {
    float sum = 0; float x, y;
    for (y = -1.5; y <= 1.5; y += 1.0)
    for (x = -1.5; x <= 1.5; x += 1.0)
    sum += shadow_offset_lookup(shadowMap, shadowCoord, vec2(x, y));

    return sum / 16.0;
}

struct DerivativesOutput
{
    vec3 db_dx;
    vec3 db_dy;
};

// Computes the partial derivatives of a triangle from the projected screen space vertices
DerivativesOutput computePartialDerivatives(vec2 v[3])
{
    DerivativesOutput result;
    float d = 1.0 / determinant(mat2(v[2] - v[1], v[0] - v[1]));
    result.db_dx = vec3(v[1].y - v[2].y, v[2].y - v[0].y, v[0].y - v[1].y) * d;
    result.db_dy = vec3(v[2].x - v[1].x, v[0].x - v[2].x, v[1].x - v[0].x) * d;

    return result;
}

vec3 interpolateAttribute(mat3 attributes, vec3 db_dx, vec3 db_dy, vec2 d)
{
    vec3 attribute_x = attributes * db_dx;;
    vec3 attribute_y = attributes * db_dy;
    vec3 attribute_s = attributes[0];

    return (attribute_s + d.x * attribute_x + d.y * attribute_y);
}

float interpolateAttribute(vec3 attributes, vec3 db_dx, vec3 db_dy, vec2 d)
{
    float attribute_x = dot(attributes, db_dx);
    float attribute_y = dot(attributes, db_dy);
    float attribute_s = attributes[0];

    return (attribute_s + d.x * attribute_x + d.y * attribute_y);
}


struct GradientInterpolationResults
{
    vec2 interp;
    vec2 dx;
    vec2 dy;
};

// Interpolate 2D attributes using the partial derivatives and generates dx and dy for texture sampling.
GradientInterpolationResults interpolateAttributeWithGradient(mat3x2 attributes, vec3 db_dx, vec3 db_dy, vec2 d, vec2 pTwoOverRes)
{
    vec3 attr0 = vec3(attributes[0][0], attributes[1][0], attributes[2][0]);
    vec3 attr1 = vec3(attributes[0][1], attributes[1][1], attributes[2][1]);;
    vec2 attribute_x = vec2(dot(db_dx, attr0), dot(db_dx, attr1));
    vec2 attribute_y = vec2(dot(db_dy, attr0), dot(db_dy, attr1));
    vec2 attribute_s = attributes[0];

    GradientInterpolationResults result;
    result.dx = attribute_x * pTwoOverRes.x;
    result.dy = attribute_y * pTwoOverRes.y;
    result.interp = (attribute_s + d.x * attribute_x + d.y * attribute_y);
    return result;
}

float depthLinearization(float depth, float near, float far)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}

void main() {


    uint v = texelFetch(visbuffer, ivec2( gl_FragCoord.xy), 0).r;

    uint drawIndex = v >> 23;
    uint triId = v & 0x007FFFFFu;

    DrawData draw = drawData[drawIndex];

    MaterialData material = materials[matId];

    uint index0 = (triId * 3 + 0) + draw.baseIndex;
    uint index1 = (triId * 3 + 1) + draw.baseIndex;
    uint index2 = (triId * 3 + 2) + draw.baseIndex;

    Vertex vertex0 = vertices[indices[index0] + draw.baseVertex];
    Vertex vertex1 = vertices[indices[index1] + draw.baseVertex];
    Vertex vertex2 = vertices[indices[index2] + draw.baseVertex];

    Transform transform = transforms[drawIndex];

    vec3 v0pos = applyTransform(vertex0.pos.xyz, transform);
    vec3 v1pos = applyTransform(vertex1.pos.xyz, transform);
    vec3 v2pos = applyTransform(vertex2.pos.xyz, transform);

    vec4 pos0 = viewProj * vec4(v0pos, 1.0);
    vec4 pos1 = viewProj * vec4(v1pos, 1.0);
    vec4 pos2 = viewProj * vec4(v2pos, 1.0);

    vec3 invw = 1.0f / vec3(pos0.w, pos1.w, pos2.w);

    pos0 *= invw[0];
    pos1 *= invw[1];
    pos2 *= invw[2];

    vec2 pos_scr[3] = { pos0.xy, pos1.xy, pos2.xy };

    DerivativesOutput derivatives = computePartialDerivatives(pos_scr);

    vec2 d = screenPos - pos_scr[0];

    float w = 1.0f / interpolateAttribute(invw, derivatives.db_dx, derivatives.db_dy, d);

    float z = w * projection[2][2] + projection[3][2];

    vec3 position = interpolateAttribute(mat3(v0pos * invw[0], v1pos * invw[1], v2pos * invw[2]), derivatives.db_dx, derivatives.db_dy, d) * w;

    vec3 viewDir = viewPos - position;

    vec3 normal0 = rotate(vertex0.normal.xyz, transform.orientation) * invw[0];
    vec3 normal1 = rotate(vertex1.normal.xyz, transform.orientation) * invw[1];
    vec3 normal2 = rotate(vertex2.normal.xyz, transform.orientation) * invw[2];

    vec3 normal = normalize(interpolateAttribute(mat3(normal0, normal1, normal2), derivatives.db_dx, derivatives.db_dy, d) * w);

    vec2 uv0 = vertex0.texcoords.xy * invw[0];
    vec2 uv1 = vertex1.texcoords.xy * invw[1];
    vec2 uv2 = vertex2.texcoords.xy * invw[2];

    GradientInterpolationResults uvInterpolation = interpolateAttributeWithGradient(mat3x2(uv0, uv1, uv2), derivatives.db_dx, derivatives.db_dy, d, 2.0f / vec2(textureSize(visbuffer, 0)));

    float linearZ = depthLinearization(z/w, 0.1, 75.0);
    float mip = pow(pow(linearZ, 0.9) * 5.0, 1.5);


    vec2 uvdx = uvInterpolation.dx * w * mip;
    vec2 uvdy = uvInterpolation.dy * w * mip;
    vec2 uv = uvInterpolation.interp * w;

    vec4 albedoAlpha = sampleImage(material.albedo, uv, vec4(material.albedoMetallicFactor.rgb, 1.0));

    PBRFragment pbr;
    pbr.albedo = albedoAlpha.rgb;
    pbr.metalicity = sampleImage(material.metallicRoughness, uv, vec4(material.albedoMetallicFactor.a)).b;
    pbr.roughness = sampleImage(material.metallicRoughness, uv, vec4(material.emissiveRoughnessFactor.a)).g;
    pbr.emmisivity = sampleImage(material.emmisive, uv, vec4(material.emissiveRoughnessFactor.rgb, 1)).rgb;

    pbr.normal = normalize(normal);
    if (material.normal != INVALID_IMAGE) {
        pbr.normal = perturb_normal(pbr.normal, viewDir, uv, texture(textures[material.normal], uv).rgb);
    }

    LightFragment light;
    light.lightDirection = -normalize(vec3(-0.75, -3, 0.35));


    vec4 shadowCoord = (shadowMat * vec4(position + normal * 0.1, 1.0));
    shadowCoord.xy = (shadowCoord.xy + 1) / 2.0f;

    light.radiance = vec3(8.0 * shadowIntensity(shadowCoord));

    vec3 col = pbrColor(pbr, light, normalize(viewDir));
    vec3 ambient =  pbr.albedo * vec3(0.09) * sampleImage(material.occlusion, uv, vec4(1)).r;

    outColor = vec4(col + ambient, 1.0);
}