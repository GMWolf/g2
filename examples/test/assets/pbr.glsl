#ifndef PBR
#define PBR

#define PI 3.14159265358979

float D_GGX(vec3 N, vec3 H, float a) {
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom = a2;
    float denom = (NdotH2*(a2- 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float G_GGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float G_smith(vec3 N, vec3 V, vec3 L, float K) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = G_GGX(NdotV, K);
    float ggx2 = G_GGX(NdotL, K);
    return ggx1 * ggx2;
}

vec3 fresnel(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0-cosTheta, 5.0);
}

struct PBRFragment {
    vec3 albedo;
    float metalicity;
    float roughness;
    vec3 emmisivity;
    vec3 normal;
};

struct LightFragment {
    vec3 lightDirection;
    vec3 radiance;
};

vec3 pbrColor(PBRFragment f, LightFragment l, vec3 viewDirection) {

    vec3 H = normalize(l.lightDirection + viewDirection);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, f.albedo, f.metalicity);

    vec3 F = fresnel(max(dot(H, viewDirection), 0.0), F0);
    float NDF = D_GGX(f.normal, H, f.roughness);
    float G = G_smith(f.normal, viewDirection, l.lightDirection, f.roughness);
    vec3 numerator = NDF * G * F;

    float denom = 4.0 * max(dot(f.normal, viewDirection), 0.0) * max(dot(f.normal, l.lightDirection), 0.0);
    vec3 specular = numerator / max(denom, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - f.metalicity;

    float NdotL = max(dot(f.normal, l.lightDirection), 0.0);

    vec3 c;
    c = (kD * f.albedo / PI + specular) * l.radiance * NdotL;
    c += f.emmisivity;
    return c;
}

    #endif //PBR