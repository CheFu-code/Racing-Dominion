#ifndef PROCEDURAL_GEOMETRY_H
#define PROCEDURAL_GEOMETRY_H

#include "Mesh.h"

namespace RacingEngine {

// Small shared mesh builder for Racing Dominion's authored circuit and vehicle.
struct Geometry {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    void triangle(const Vector3& a, const Vector3& b, const Vector3& c) {
        Vector3 n = Vector3::cross(b - a, c - a).normalized();
        unsigned int base = static_cast<unsigned int>(vertices.size());
        for (const auto& p : {a, b, c})
            vertices.push_back({{p.x, p.y, p.z}, {n.x, n.y, n.z}, {0, 0}});
        indices.insert(indices.end(), {base, base + 1, base + 2});
    }

    void quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) {
        Vector3 n = Vector3::cross(b - a, c - a).normalized();
        unsigned int base = static_cast<unsigned int>(vertices.size());
        const Vector3 points[] = {a, b, c, d};
        for (int i = 0; i < 4; ++i) {
            vertices.push_back({{points[i].x, points[i].y, points[i].z},
                                {n.x, n.y, n.z}, {i == 1 || i == 2 ? 1.f : 0.f, i > 1 ? 1.f : 0.f}});
        }
        indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    void box(const Vector3& center, const Vector3& size, float frontWidth = 1.f) {
        float x = size.x * 0.5f, y = size.y * 0.5f, z = size.z * 0.5f;
        Vector3 a = center + Vector3(-x, -y, -z);
        Vector3 b = center + Vector3(x, -y, -z);
        Vector3 c = center + Vector3(x * frontWidth, -y, z);
        Vector3 d = center + Vector3(-x * frontWidth, -y, z);
        Vector3 up(0, size.y, 0);
        quad(a + up, d + up, c + up, b + up);
        quad(d, c, c + up, d + up);
        quad(b, a, a + up, b + up);
        quad(c, b, b + up, c + up);
        quad(a, d, d + up, a + up);
        quad(a, b, c, d);
    }

    void rotatedBox(const Vector3& center, const Vector3& size, float yaw) {
        Geometry local;
        local.box(Vector3(), size);
        unsigned int base = static_cast<unsigned int>(vertices.size());
        float c = cosf(yaw), s = sinf(yaw);
        for (auto vertex : local.vertices) {
            float x = vertex.position[0], z = vertex.position[2];
            vertex.position[0] = center.x + x * c + z * s;
            vertex.position[1] += center.y;
            vertex.position[2] = center.z - x * s + z * c;
            x = vertex.normal[0]; z = vertex.normal[2];
            vertex.normal[0] = x * c + z * s;
            vertex.normal[2] = -x * s + z * c;
            vertices.push_back(vertex);
        }
        for (unsigned int index : local.indices) indices.push_back(base + index);
    }

    void wheel(const Vector3& center, float radius, float width) {
        for (int i = 0; i < 16; ++i) {
            float a = i * 6.2831853f / 16, b = (i + 1) * 6.2831853f / 16;
            Vector3 p(0, sinf(a) * radius, cosf(a) * radius);
            Vector3 q(0, sinf(b) * radius, cosf(b) * radius);
            Vector3 left = center - Vector3(width * 0.5f, 0, 0);
            Vector3 right = center + Vector3(width * 0.5f, 0, 0);
            quad(left + p, right + p, right + q, left + q);
            triangle(left, left + p, left + q);
            triangle(right, right + q, right + p);
        }
    }

    Mesh* upload() const {
        Mesh* mesh = new Mesh();
        mesh->upload(vertices, indices);
        return mesh;
    }
};

} // namespace RacingEngine
#endif
