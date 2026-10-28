// Host tests run the actual course/car generators, replacing GPU uploads only.
#include <cassert>
#include <cmath>
#include <iostream>
#include "Track.h"
#include "DominionCar.h"
#include "Shader.h"
#include "Vehicle.h"
#include "CircuitChallenge.h"
#include "SmokeTrail.h"
#include "Engine.h"
#include "AssetLoader.h"
#include <limits>

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
void Shader::setFloat(const char*, float) const {}
void Shader::setInt(const char*, int) const {}
Shader::Shader() : programId(0) {}
Shader::~Shader() = default;
Renderer::Renderer() = default;
Renderer::~Renderer() = default;
bool Renderer::initialize() { return true; }
void Renderer::shutdown() {}
void Renderer::resize(int, int) {}
void Renderer::setClearColor(float, float, float, float) {}
void Renderer::beginFrame() {}
void Renderer::endFrame() {}
bool AssetLoader::initialize(AAssetManager*) { return true; }
}

int main() {
    assert(CircuitChallenge::rank(8, 8) == 3);
    assert(CircuitChallenge::rank(10.5f, 8) == 2);
    assert(CircuitChallenge::rank(14, 8) == 1);
    assert(CircuitChallenge::rank(14.01f, 8) == 0);
    assert(CircuitChallenge::rank(0, 8) == 0);
    assert(CircuitChallenge::rank(std::numeric_limits<float>::quiet_NaN(), 8) == 0);
    CircuitChallenge challenge;
    assert(!challenge.cross(1)); // Zero-time gate cannot award a sector.
    challenge.tick(-1); challenge.tick(std::numeric_limits<float>::infinity());
    assert(challenge.raceTime == 0);
    for (int i = 0; i < 12; ++i) {
        challenge.tick(7);
        assert(!challenge.cross((i + 2) % 4)); // Reject out-of-order gates.
        assert(challenge.cross((i + 1) % 4));
        assert(!challenge.cross((i + 1) % 4)); // Holding a gate does not award again.
    }
    assert(challenge.finished() && challenge.claimed() == 12 && challenge.points() == 36);
    assert(challenge.dominionRank() == 3 && challenge.bestLap == 28 && challenge.raceTime == 84);
    challenge.tick(10);
    assert(challenge.raceTime == 84 && !challenge.cross(1));
    challenge.reset(); assert(challenge.points() == 0 && challenge.completed == 0 && challenge.raceTime == 0);
    for (int tier = 0; tier < 4; ++tier) {
        challenge.reset();
        for (int i = 0; i < 12; ++i) { challenge.tick(tier == 3 ? 7 : tier == 2 ? 10 : tier == 1 ? 13 : 20); challenge.cross((i + 1) % 4); }
        assert(challenge.dominionRank() == tier);
    }
    challenge.medals.fill(2);
    for (int i = 0; i < 6; ++i) challenge.medals[i] = 3;
    assert(challenge.points() == 30 && challenge.dominionRank() == 3);
    challenge.medals[0] = 2; assert(challenge.dominionRank() == 2);
    challenge.medals.fill(0);
    for (int i = 0; i < 10; ++i) challenge.medals[i] = 2;
    assert(challenge.points() == 20 && challenge.dominionRank() == 2);
    challenge.medals[0] = 1; assert(challenge.dominionRank() == 1);
    for (int i = 0; i < 10; ++i) challenge.medals[i] = 1;
    assert(challenge.points() == 10 && challenge.dominionRank() == 1);
    challenge.medals[0] = 0; assert(challenge.dominionRank() == 0);
    SmokeTrail trail;
    trail.reset();
    for (int i = 0; i < 400; ++i) trail.update(Vector3(), 0.4f, 1, 1.f / 60);
    int active = 0;
    for (const auto& puff : trail.puffs) { if (puff.age < 1.1f) ++active; assert(std::isfinite(puff.position.y)); }
    assert(active > 0 && active <= 32);
    for (int i = 0; i < 120; ++i) trail.update(Vector3(), 0, 0, 1.f / 60);
    for (const auto& puff : trail.puffs) assert(puff.age > 1.1f);
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
    turning.hitBarrier(Vector3(1, 0.5f, 2), 0);
    assert(turning.getPosition().x == 1 && turning.getSpeed() < at30.getSpeed() * 0.21f);
    assert(turning.getVelocity().x == 0 && turning.getVelocity().z > 0);
    {
        Track track;
        track.generateDefaultTrack();
        assert(liveMeshes == 8);
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
            const auto& segment = segments[gate * segments.size() / 4];
            assert(track.getCheckpointIndex(segment.center) == gate);
            Vector3 tangent(sinf(segment.rotation), 0, cosf(segment.rotation));
            Vector3 lateral(cosf(segment.rotation), 0, -sinf(segment.rotation));
            Vector3 a = segment.center - tangent * 10, b = segment.center + tangent * 10;
            assert(track.crossedCheckpoint(a, b) == gate); // Swept crossing at speed.
            assert(track.crossedCheckpoint(b, a) == -1); // No reverse awards.
            assert(track.crossedCheckpoint(a + lateral * 12, b + lateral * 12) == -1);
        }
        assert(track.getCheckpointIndex(Vector3(0, 0, 0)) == -1);
        const auto& first = segments.front();
        Vector3 lateral(cosf(first.rotation), 0, -sinf(first.rotation));
        assert(track.checkCollision(first.center + lateral * 6.f, 1.5f));
        track.generateDefaultTrack();
        assert(liveMeshes == 8); // Rebuilding releases the previous GPU resources.
        DominionCar car;
        car.initialize();
        assert(liveMeshes == 13);
        car.setPosition(track.getStartPosition());
        car.setRotation(track.getStartRotation(), Vector3(0, 1, 0));
        car.initialize();
        assert(liveMeshes == 13);
        car.destroy();
        assert(liveMeshes == 8);
    }
    assert(liveMeshes == 0);
    {
        Engine engine;
        assert(engine.initialize(nullptr));
        engine.resize(800, 400);
        float data[34]{};
        engine.writeTelemetry(data); assert(data[3] == 0);
        engine.action(0);
        for (int i = 0; i < 120; ++i) engine.update(1.f / 60);
        engine.writeTelemetry(data); assert(data[3] == 5 && data[2] == 0);
        float remaining = data[4];
        engine.action(1);
        for (int i = 0; i < 100; ++i) engine.update(1.f / 60);
        engine.writeTelemetry(data); assert(data[3] == 3 && data[4] == remaining && data[2] == 0);
        engine.action(0);
        for (int i = 0; i < 70; ++i) engine.update(1.f / 60);
        engine.writeTelemetry(data); assert(data[3] == 2);
        engine.setControls(0, 1, 0);
        for (int i = 0; i < 60; ++i) engine.update(1.f / 60);
        engine.writeTelemetry(data); assert(data[0] > 5);
        engine.action(2); engine.update(1.f / 60);
        engine.writeTelemetry(data); float pausedTime = data[2];
        assert(data[3] == 3 && data[19] == 0 && data[20] == 0 && data[21] == 0);
        engine.action(2); engine.update(1.f / 60); // Repeated background request stays paused.
        engine.writeTelemetry(data); assert(data[3] == 3 && data[2] == pausedTime);
        engine.action(0); engine.update(1.f / 60);
        engine.writeTelemetry(data); assert(data[3] == 2 && data[2] > pausedTime);
        engine.render();
    }
    assert(liveMeshes == 0);
    std::cout << "Race states, sector medals, smoke lifetime, acceleration, braking, steering, gate crossings, course geometry, and cleanup passed ("
              << uploads << " mesh uploads).\n";
}
