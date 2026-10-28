#ifndef PROCEDURAL_GEOMETRY_H
#define PROCEDURAL_GEOMETRY_H

#include "Mesh.h"

namespace RacingEngine {

// Small shared mesh builder for Racing Dominion's authored circuit and vehicle.
struct Geometry {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

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

    Mesh* upload() const {
        Mesh* mesh = new Mesh();
        mesh->upload(vertices, indices);
        return mesh;
    }
};

} // namespace RacingEngine
#endif
