//
// Created by felix on 26/09/2021.
//

#include <vector>
#include <span>
#include "sdf_compiler.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <cstring>

static const char* GLTF_ATTRIBUTE_POSITION = "POSITION";


static float point_segment_distance(const glm::vec3& x0, const glm::vec3& x1, const glm::vec3& x2) {
    glm::vec3 dx = x2 - x1;
    float m2 = glm::dot(dx, dx);

    float s12 = glm::dot(x2 - x0, dx) / m2;

    s12 = std::clamp(s12, 0.0f, 1.0f);

    return glm::distance(x0, s12*x1+(1-s12)*x2);
}

static float point_triangle_distance(const glm::vec3& x0, const glm::vec3& x1, const glm::vec3& x2, const glm::vec3& x3){

    glm::vec3 x13 = x1 - x3;
    glm::vec3 x23 = x2 - x3;
    glm::vec3 x03 = x0 - x3;
    float m13 = glm::dot(x13, x13);
    float m23 = glm::dot(x23, x23);
    float d = glm::dot(x13, x23);
    float invdet = 1.0f/std::max(m13*m23-d*d, 1e-30f);
    float a = glm::dot(x13, x03);
    float b = glm::dot(x23, x03);

    float w23 = invdet*(m23*a-d*b);
    float w31 = invdet*(m13*b-d*a);
    float w12 = 1-w23-w31;
    if (w23>=0 && w31>=0 && w12>=0) {
        return glm::distance(x0, w23*x1+w31*x2+w12*x3);
    } else {
        if (w23 > 0)
            return std::min(point_segment_distance(x0, x1, x2), point_segment_distance(x0, x1, x3));
        else if(w31>0)
            return std::min(point_segment_distance(x0, x1, x2), point_segment_distance(x0, x2, x3));
        else
            return std::min(point_segment_distance(x0, x1, x3), point_segment_distance(x0, x2, x3));
    }
}

static int orientation(glm::vec2 v1, glm::vec2 v2, float& sarea)
{
    sarea = v1.y*v2.x-v1.x*v2.y;
    if (sarea>0)return 1;
    else if (sarea<0) return -1;
    else if (v2.y > v1.y) return 1;
    else if (v2.y < v1.y) return -1;
    else if (v1.x > v2.x) return 1;
    else if (v1.x < v2.x) return -1;
    else return 0;
}

static bool point_in_triangle_2d(glm::vec2 v0, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, glm::vec3& a)
{
    v1 -= v0;
    v2 -= v0;
    v3 -= v0;
    int signa = orientation(v2, v3, a.x);
    if (signa == 0) return false;
    int signb = orientation(v3, v1, a.y);
    if (signb !=signa) return false;
    int signc= orientation(v1, v2, a.z);
    if (signc != signa) return false;
    float sum = a.x + a.y + a.z;
    a /= sum;
    return true;
}

static void check_neighbour(const std::vector<glm::uvec3>& tris, const std::vector<glm::vec3>& positions,
                            std::vector<float>& sdf, std::vector<int>& closest_tri, int width, int height, int depth,
                            const glm::vec3& gx, int i0, int j0, int k0, int i1, int j1, int k1) {
    auto getIndex = [width, height, depth](int i, int j, int k) {
        return k * width * height + j * width + i;
    };

    if (closest_tri[getIndex(i1, j1, k1)] >=0) {
        auto t = tris[closest_tri[getIndex(i1, j1, k1)]];
        float d = point_triangle_distance(gx, positions[t.x], positions[t.y], positions[t.z]);
        if (d < sdf[getIndex(i0, j0, k0)]) {
            sdf[getIndex(i0, j0, k0)] = d;
            closest_tri[getIndex(i0,j0,k0)] = closest_tri[getIndex(i1, j1, k1)];
        }

    }


}

static void sweep(const std::vector<glm::uvec3>& tris, const std::vector<glm::vec3>& positions,
                  std::vector<float>& sdf, std::vector<int>& closest_tri, const glm::vec3& origin, float dx,
                  glm::ivec3 d, int width, int height, int depth) {



    int i0, i1;
    if (d.x > 0) {
        i0=1;
        i1 = width;
    } else {
        i0 = width - 2;
        i1 = -1;
    }
    int j0, j1;
    if (d.y > 0) {
        j0 = 1;
        j1 = height;
    } else {
        j0 = height - 2;
        j1 = -1;
    }
    int k0, k1;
    if (d.z > 0) {
        k0 = 1;
        k1 = depth;
    } else {
        k0 = depth-2;
        k1 = -1;
    }

    for(int k = k0; k!=k1; k+=d.z)
    for(int j = j0; j!=j1; j+=d.y)
    for(int i = i0; i!=i1; i+=d.x) {
        glm::vec3 gx = glm::vec3(i,j,k) * dx + origin;
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i-d.x, j, k);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i, j-d.y, k);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i-d.x, j-d.y, k);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i, j, k-d.z);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i-d.x, j, k-d.z);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i, j-d.y, k-d.z);
        check_neighbour(tris, positions, sdf, closest_tri, width, height, depth, gx, i, j, k, i-d.x, j-d.y, k-d.z);
    }
}
std::vector<float> genSDF(const cgltf_mesh *mesh, int width, int height, int depth, float dx, glm::vec3 origin, int exact_band) {


    auto getIndex = [width, height, depth](int i, int j, int k) {
        return k * width * height + j * width + i;
    };

    //get all tris
    std::vector<glm::uvec3> tris;
    std::vector<glm::vec3> positions;
    for(const cgltf_primitive& primitive : std::span(mesh->primitives, mesh->primitives_count)) {
        for(uint i = 0; i < primitive.indices->count; i+=3) {
            glm::uvec3 t;
            t.x = positions.size() + cgltf_accessor_read_index(primitive.indices, i + 0);
            t.y = positions.size() + cgltf_accessor_read_index(primitive.indices, i + 1);
            t.z = positions.size() + cgltf_accessor_read_index(primitive.indices, i + 2);
            tris.push_back(t);
        }

        for(const cgltf_attribute& attribute : std::span(primitive.attributes, primitive.attributes_count)) {
            if (strcmp(attribute.name, GLTF_ATTRIBUTE_POSITION) == 0) {
                for(int vertexIndex = 0; vertexIndex < attribute.data->count; vertexIndex++) {
                    glm::vec3 p = positions.emplace_back();
                    cgltf_accessor_read_float(attribute.data, vertexIndex, &p.x, 3);
                }
            }
        }
    }

    std::vector<float> sdf(width * height * depth, float(width + height + depth) * dx);


    std::vector<int> closest_tri(width * height * depth, -1);
    std::vector<int> intersection_count(width * height * depth, 0);

    for(uint t = 0; t < tris.size(); t++) {

        glm::vec3 fp = (positions[tris[t].x] - origin) / dx;
        glm::vec3 fq = (positions[tris[t].y] - origin) / dx;
        glm::vec3 fr = (positions[tris[t].z] - origin) / dx;

        glm::ivec3 v0 = glm::clamp(glm::ivec3(glm::min(glm::min(fp, fq), fr)) - exact_band, glm::ivec3(0), glm::ivec3(width-1, height-1, depth-1));
        glm::ivec3 v1 = glm::clamp(glm::ivec3(glm::max(glm::max(fp, fq), fr)) + exact_band + 1, glm::ivec3(0), glm::ivec3(width-1, height-1, depth-1));

        for(int k = v0.z; k < v1.z; ++k)
        for(int j = v0.y; j < v1.y; ++j)
        for(int i = v0.x; i < v1.x; ++i) {
            glm::vec3 gx = glm::vec3(i, j, k) * dx + origin;
            float d = point_triangle_distance(gx, positions[tris[t].x], positions[tris[t].y], positions[tris[t].z]);
            if (d < sdf[getIndex(i,j,k)]) {
                sdf[getIndex(i,j,k)] = d;
                closest_tri[getIndex(i,j,k)] = t;
            }
        }

        v0.y = std::clamp((int)std::ceil(std::min(fp.y, std::min(fq.y, fr.y))), 0, height - 1);
        v1.y = std::clamp((int)std::floor(std::max(fp.y, std::max(fq.y, fr.y))), 0, height - 1);
        v0.z = std::clamp((int)std::ceil(std::min(fp.z, std::min(fq.z, fr.z))), 0, depth - 1);
        v1.z = std::clamp((int)std::floor(std::max(fp.z, std::max(fq.z, fr.z))), 0, depth - 1);
        for(int k = v0.z; k < v1.z; ++k)
        for(int j = v0.y; j < v1.y; ++j) {
            glm::vec3 a;
            if (point_in_triangle_2d(glm::vec2(j,k), glm::vec2(fp.y, fp.z), glm::vec2(fq.y, fq.z), glm::vec2(fr.y, fr.z), a)) {
                float fi = a.x * fp.x + a.y * fq.x + a.z * fr.x;
                int i_interval = int(std::ceil(fi));
                if (i_interval < 0) ++intersection_count[getIndex(0, j, k)];
                else if (i_interval<width)++intersection_count[getIndex(i_interval, j, k)];
            }
        }
    }

    for(uint pass = 0; pass<2; pass++) {
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(+1, +1, +1), width, height, depth);        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(+1, +1, +1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(-1, -1, -1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(+1, +1, -1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(-1, -1, +1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(+1, -1, +1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(-1, +1, -1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(+1, -1, -1), width, height, depth);
        sweep(tris, positions, sdf, closest_tri, origin, dx, glm::vec3(-1, +1, +1), width, height, depth);
    }


    for(int k = 0; k < depth; k++)
    for(int j = 0; j < height; j++) {
        int total_count = 0;
        for(int i = 0; i < width; i++) {
            total_count += intersection_count[getIndex(i,j,k)];
            if(total_count%2==1) {
                sdf[getIndex(i,j,k)] *= -1;
            }
        }
    }

    return sdf;
}
