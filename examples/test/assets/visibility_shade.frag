#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

#define UINT32_MAX    (4294967295U)
#define INVALID_IMAGE UINT32_MAX

#include "pbr.glsl"
#include "normals.glsl"
#include "scene.glsl"
#include "transform.glsl"
#include "geometry.glsl"
#include "drawData.glsl"

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


layout( push_constant ) uniform PushConstant {
    uint matId;
};

layout(set = 1, binding = 2) buffer TransformBlock {
    Transform transforms[];
};

layout(early_fragment_tests) in;

layout(location = 0) out vec4 outColor;

vec4 sampleImage(uint imageIndex, vec2 uv, vec4 def, vec2 uvdx, vec2 uvdy) {
    if (imageIndex == INVALID_IMAGE) {
        return def;
    } else {
        //return texture(textures[imageIndex], uv);
        return textureGrad(textures[imageIndex], uv, uvdx, uvdy);
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
    vec3 attribute_x = attributes * db_dx;
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

    result.dx.x = dot((db_dx * pTwoOverRes.x) * attr0, vec3(1));
    result.dx.y = dot((db_dy * pTwoOverRes.y) * attr0, vec3(1));
    result.dy.x = dot((db_dx * pTwoOverRes.x) * attr1, vec3(1));
    result.dy.y = dot((db_dy * pTwoOverRes.y) * attr1, vec3(1));
    return result;
}

float depthLinearization(float depth, float near, float far)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}


struct BarycentricDeriv
{
    vec3 m_lambda;
    vec3 m_ddx;
    vec3 m_ddy;
};


BarycentricDeriv CalcFullBary(vec4 pt0, vec4 pt1, vec4 pt2, vec2 pixelNdc, vec2 winSize)
{
    BarycentricDeriv ret;

    vec3 invW = 1.0 / (vec3(pt0.w, pt1.w, pt2.w));

    vec2 ndc0 = pt0.xy * invW.x;
    vec2 ndc1 = pt1.xy * invW.y;
    vec2 ndc2 = pt2.xy * invW.z;

    float invDet = 1.0 / determinant(mat2(ndc2 - ndc1, ndc0 - ndc1));
    ret.m_ddx = vec3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet;
    ret.m_ddy = vec3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet;

    vec2 deltaVec = pixelNdc - ndc0;

    float interpInvW = (invW.x + deltaVec.x * dot(invW, ret.m_ddx) + deltaVec.y * dot(invW, ret.m_ddy));
    float interpW = 1.0 / interpInvW;

    ret.m_lambda.x = interpW * (invW[0] + deltaVec.x * ret.m_ddx.x * invW[0] + deltaVec.y * ret.m_ddy.x * invW[0]);
    ret.m_lambda.y = interpW * (0.0f    + deltaVec.x * ret.m_ddx.y * invW[1] + deltaVec.y * ret.m_ddy.y * invW[1]);
    ret.m_lambda.z = interpW * (0.0f    + deltaVec.x * ret.m_ddx.z * invW[2] + deltaVec.y * ret.m_ddy.z * invW[2]);


    ret.m_ddx *= (2.0f / winSize.x);
    ret.m_ddy *= (2.0f / winSize.y);

    ret.m_ddy *= -1.0f;

    return ret;
}

vec3 InterpolateWithDeriv(BarycentricDeriv deriv, vec3 v)
{
    vec3 ret = vec3(0);

    ret.x = dot(deriv.m_lambda, v);
    ret.y = dot(deriv.m_ddx * v, vec3(1));
    ret.z = dot(deriv.m_ddy * v, vec3(1));

    return ret;
}

void main() {
    uint v = texelFetch(visbuffer, ivec2(gl_FragCoord.xy ), 0).r - 1;
    uint drawIndex = v >> 7;
    uint triId = v & 0x7Fu;

    DrawData draw = drawData[drawIndex];

    //if (draw.materialIndex != matId) {
    //    discard;
    //}

    MaterialData material = materials[matId];

    uint index0 = indices[(triId * 3 + 0) + draw.baseIndex];
    uint index1 = indices[(triId * 3 + 1) + draw.baseIndex];
    uint index2 = indices[(triId * 3 + 2) + draw.baseIndex];

    vec3 inPos0 = loadPosition(draw.positionOffset, index0);
    vec3 inPos1 = loadPosition(draw.positionOffset, index1);
    vec3 inPos2 = loadPosition(draw.positionOffset, index2);

    Transform transform = transforms[drawIndex];

    vec3 v0pos = applyTransform(inPos0, transform);
    vec3 v1pos = applyTransform(inPos1, transform);
    vec3 v2pos = applyTransform(inPos2, transform);

    vec4 pos0 = viewProj * vec4(v0pos, 1.0);
    vec4 pos1 = viewProj * vec4(v1pos, 1.0);
    vec4 pos2 = viewProj * vec4(v2pos, 1.0);


    BarycentricDeriv baryDeriv = CalcFullBary(pos0, pos1, pos2, screenPos, 2 * vec2(textureSize(visbuffer, 0)));


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

    vec3 inNormal0 = loadNormal(draw.normalOffset, index0);
    vec3 inNormal1 = loadNormal(draw.normalOffset, index1);
    vec3 inNormal2 = loadNormal(draw.normalOffset, index2);

    vec3 normal0 = rotate(inNormal0, transform.orientation) * invw[0];
    vec3 normal1 = rotate(inNormal1, transform.orientation) * invw[1];
    vec3 normal2 = rotate(inNormal2, transform.orientation) * invw[2];

    vec3 normal = normalize(interpolateAttribute(mat3(normal0, normal1, normal2), derivatives.db_dx, derivatives.db_dy, d) * w);

    vec3 inTangent0 = loadTangent(draw.tangentOffset, index0);
    vec3 inTangent1 = loadTangent(draw.tangentOffset, index1);
    vec3 inTangent2 = loadTangent(draw.tangentOffset, index2);

    vec3 tangent0 = rotate(inTangent0, transform.orientation) * invw[0];
    vec3 tangent1 = rotate(inTangent1, transform.orientation) * invw[1];
    vec3 tangent2 = rotate(inTangent2, transform.orientation) * invw[2];

    vec3 tangent = normalize(interpolateAttribute(mat3(tangent0, tangent1, tangent2), derivatives.db_dx, derivatives.db_dy, d) * w);

    vec3 inBiTangent0 = loadBiTangent(draw.bitangentOffset, index0);
    vec3 inBiTangent1 = loadBiTangent(draw.bitangentOffset, index1);
    vec3 inBiTangent2 = loadBiTangent(draw.bitangentOffset, index2);

    vec3 bitangent0 = rotate(inBiTangent0, transform.orientation) * invw[0];
    vec3 bitangent1 = rotate(inBiTangent1, transform.orientation) * invw[1];
    vec3 bitangent2 = rotate(inBiTangent2, transform.orientation) * invw[2];

    vec3 bitangent = normalize(interpolateAttribute(mat3(bitangent0, bitangent1, bitangent2), derivatives.db_dx, derivatives.db_dy, d) * w);

    mat3 tbn = mat3(tangent, bitangent, normal);


    //normal.x = InterpolateWithDeriv(baryDeriv, vec3(normal0.x, normal1.x, normal2.x)).x;
    //normal.y = InterpolateWithDeriv(baryDeriv, vec3(normal0.y, normal1.y, normal2.y)).x;
    //normal.z = InterpolateWithDeriv(baryDeriv, vec3(normal0.z, normal1.z, normal2.z)).x;

    vec2 inUv0 = loadTexcoord(draw.texcoordOffset, index0);
    vec2 inUv1 = loadTexcoord(draw.texcoordOffset, index1);
    vec2 inUv2 = loadTexcoord(draw.texcoordOffset, index2);

    vec2 uv0 = inUv0 * invw[0];
    vec2 uv1 = inUv1 * invw[1];
    vec2 uv2 = inUv2 * invw[2];

    GradientInterpolationResults uvInterpolation = interpolateAttributeWithGradient(mat3x2(uv0, uv1, uv2), derivatives.db_dx, derivatives.db_dy, d, 2.0f / vec2(textureSize(visbuffer, 0)));

    float linearZ = depthLinearization(z/w, 0.1, 75.0);
    float mip = 1;// pow(pow(linearZ, 0.9) * 5.0, 1.5);

    vec2 uvdx = uvInterpolation.dx * w * mip;
    vec2 uvdy = uvInterpolation.dy * w * mip;
    vec2 uv = uvInterpolation.interp * w;

    //vec3 uvix = InterpolateWithDeriv(baryDeriv, vec3(inUv0.x, inUv1.x, inUv2.x));
    //vec3 uviy = InterpolateWithDeriv(baryDeriv, vec3(inUv0.y, inUv1.y, inUv2.y));
//
    //uv.x = uvix.x;
    //uv.y = uviy.x;
//
    //uvdx = uvix.yz;
    //uvdy = uviy.yz;

    vec4 albedoAlpha = sampleImage(material.albedo, uv, vec4(material.albedoMetallicFactor.rgb, 1.0), uvdx, uvdy);

    PBRFragment pbr;
    pbr.albedo = albedoAlpha.rgb;
    pbr.metalicity = sampleImage(material.metallicRoughness, uv, vec4(material.albedoMetallicFactor.a), uvdx, uvdy).b;
    pbr.roughness = sampleImage(material.metallicRoughness, uv, vec4(material.emissiveRoughnessFactor.a), uvdx, uvdy).g;
    pbr.emmisivity = sampleImage(material.emmisive, uv, vec4(material.emissiveRoughnessFactor.rgb, 1), uvdx, uvdy).rgb;

    pbr.normal = normal;
    if (material.normal != INVALID_IMAGE) {
        //pbr.normal = perturb_normal(pbr.normal, viewDir, uv, texture(textures[material.normal], uv).rgb);
        pbr.normal = normalize(tbn * (texture(textures[material.normal], uv).rgb * 2.0 - 1.0));
    }

    LightFragment light;
    light.lightDirection = -normalize(vec3(-0.75, 0.35, 3));


    vec4 shadowCoord = (shadowMat * vec4(position + normal * 0.1, 1.0));
    shadowCoord.xy = (shadowCoord.xy + 1) / 2.0f;

    light.radiance = vec3(8.0 * shadowIntensity(shadowCoord));

    vec3 col = pbrColor(pbr, light, normalize(viewDir));
    vec3 ambient =  pbr.albedo * vec3(0.05) * sampleImage(material.occlusion, uv, vec4(1), uvdx, uvdy).r;

    outColor = vec4(col + ambient, 1.0);
}