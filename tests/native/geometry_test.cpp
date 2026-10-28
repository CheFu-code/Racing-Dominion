// Host tests run the actual course/car generators, replacing GPU uploads only.
#include <cassert>
#include <cmath>
#include <iostream>
#include "Track.h"
#include "DominionCar.h"
#include "Shader.h"
#include "Vehicle.h"

using namespace RacingEngine;
static int liveMeshes = 0;
static int uploads = 0;

namespace RacingEngine {
Mesh::Mesh() : vao(0), vbo(0), ebo(0), indexCount(0) {}
Mesh::~Mesh() { destroy(); }
void Mesh::destroy() { if (vao) --liveMeshes; vao = 0; }
void Mesh::draw() const { assert(vao != 0); }
void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    assert(!vertices.empty() && !indices.empty() && indices.size() % 3 == 0);
    for (const auto& vertex : vertices) {
        for (float coordinate : vertex.position) assert(std::isfinite(coordinate));
        for (float normal : vertex.normal) assert(std::isfinite(normal));
    }
    for (size_t i = 0; i < indices.size(); i += 3) {
        for (size_t j = i; j < i + 3; ++j) assert(indices[j] < vertices.size());
        auto position = [&vertices](unsigned int n) {
            const auto& p = vertices[n].position;
            return Vector3(p[0], p[1], p[2]);
        };
        Vector3 a = position(indices[i]), b = position(indices[i + 1]), c = position(indices[i + 2]);
        Vector3 cross = Vector3::cross(b - a, c - a);
        assert(cross.length() > 0.00001f); // No degenerate triangles.
        const auto& n = vertices[indices[i]].normal;
        assert(Vector3::dot(cross, Vector3(n[0], n[1], n[2])) > 0);
    }
    if (!vao) ++liveMeshes;
    vao = 1;
    ++uploads;
}
Model::Model() : rotationAngle(0), rotationAxis(0, 1, 0), scale(1, 1, 1) {}
Model::~Model() { destroy(); }
void Model::addMesh(Mesh* mesh, unsigned int texture) { meshes.push_back({mesh, texture}); }
void Model::destroy() { for (auto& entry : meshes) delete entry.mesh; meshes.clear(); }
void Model::draw(const Shader&) const { for (const auto& entry : meshes) entry.mesh->draw(); }
void Shader::setVec3(const char*, float, float, float) const {}
void Shader::setMat4(const char*, const float*) const {}
}

int main() {
    auto accelerate = [](int fps) {
        Vehicle vehicle;
        for (int frame = 0; frame < fps * 5; ++frame) {
            vehicle.applyAcceleration(1);
            vehicle.update(1.f / fps);
        }
        return vehicle;
    };
    Vehicle at30 = accelerate(30), at60 = accelerate(60);
    assert(at60.getSpeed() > 15.f); // Reach a useful racing speed within five seconds.
    assert(fabsf(at30.getSpeed() - at60.getSpeed()) < 0.5f);
    Vehicle turning = at60;
    turning.applySteering(0.8f, 1.f / 60);
    assert(turning.getVelocity().x < 0); // The upper-left zone steers left.
    Vehicle angled;
    angled.setHeading(-0.5f);
    angled.applyAcceleration(1);
    angled.update(1.f / 60);
    assert(angled.getVelocity().x < 0 && angled.getVelocity().z > 0);
    for (int frame = 0; frame < 60 * 4; ++frame) {
        at60.applyBrake(1);
        at60.update(1.f / 60);
        assert(at60.getVelocity().z >= 0); // Braking must never engage reverse.
    }
    assert(at60.getSpeed() < 0.01f);
    {
        Track track;
        track.generateDefaultTrack();
        assert(liveMeshes == 5);
        const auto& segments = track.getSegments();
        assert(segments.size() >= 64);
        for (size_t i = 0; i < segments.size(); ++i) {
            const auto& a = segments[i];
            const auto& b = segments[(i + 1) % segments.size()];
            Vector3 end = a.center + Vector3(sinf(a.rotation), 0, cosf(a.rotation)) * (a.length * 0.5f);
            Vector3 start = b.center - Vector3(sinf(b.rotation), 0, cosf(b.rotation)) * (b.length * 0.5f);
            assert((end - start).length() < 0.001f); // Closed course, including the final seam.
            assert(track.isOnTrack(a.center));
            assert(!track.checkCollision(a.center, 1.5f));
        }
        assert(track.isOnTrack(track.getStartPosition()));
        assert(!track.isOnTrack(Vector3(0, 0, 0)));
        for (int gate = 0; gate < 4; ++gate) {
            assert(track.getCheckpointIndex(segments[gate * segments.size() / 4].center) == gate);
        }
        assert(track.getCheckpointIndex(Vector3(0, 0, 0)) == -1);
        const auto& first = segments.front();
        Vector3 lateral(cosf(first.rotation), 0, -sinf(first.rotation));
        assert(track.checkCollision(first.center + lateral * 6.f, 1.5f));
        track.generateDefaultTrack();
        assert(liveMeshes == 5); // Rebuilding releases the previous GPU resources.
        DominionCar car;
        car.initialize();
        assert(liveMeshes == 9);
        car.setPosition(track.getStartPosition());
        car.setRotation(track.getStartRotation(), Vector3(0, 1, 0));
        car.initialize();
        assert(liveMeshes == 9);
        car.destroy();
        assert(liveMeshes == 5);
    }
    assert(liveMeshes == 0);
    std::cout << "Acceleration, braking, steering, frame rates, course continuity, road bounds, four gates, mesh triangles, and cleanup passed ("
              << uploads << " mesh uploads).\n";
}
