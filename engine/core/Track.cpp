#include "Track.h"
#include "Shader.h"
#include "ProceduralGeometry.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace RacingEngine {
namespace {
constexpr float PI = 3.14159265f;

Vector3 forward(const TrackSegment& segment) {
    return Vector3(sinf(segment.rotation), 0, cosf(segment.rotation));
}

Vector3 right(const TrackSegment& segment) {
    return Vector3(cosf(segment.rotation), 0, -sinf(segment.rotation));
}

// Shared vertices at each joint keep the road and its markings watertight.
Vector3 joint(const std::vector<TrackSegment>& segments, size_t index, float offset, float y) {
    size_t previous = (index + segments.size() - 1) % segments.size();
    const auto& segment = segments[index];
    Vector3 lateral = (right(segments[previous]) + right(segment)).normalized();
    float miter = fmaxf(0.5f, Vector3::dot(lateral, right(segment)));
    Vector3 p = segment.center - forward(segment) * (segment.length * 0.5f);
    return p + lateral * (offset / miter) + Vector3(0, y, 0);
}

void strip(Geometry& mesh, const std::vector<TrackSegment>& segments,
           size_t i, float left, float rightOffset, float y) {
    size_t next = (i + 1) % segments.size();
    mesh.quad(joint(segments, i, left, y), joint(segments, next, left, y),
              joint(segments, next, rightOffset, y), joint(segments, i, rightOffset, y));
}

float roadDistance(const std::vector<TrackSegment>& segments, const Vector3& position) {
    float nearest = std::numeric_limits<float>::max();
    for (const auto& segment : segments) {
        Vector3 diff = position - segment.center;
        Vector3 tangent = forward(segment);
        float along = Vector3::dot(diff, tangent);
        along = fmaxf(-segment.length * 0.5f, fminf(segment.length * 0.5f, along));
        Vector3 delta = diff - tangent * along;
        nearest = fminf(nearest, delta.x * delta.x + delta.z * delta.z);
    }
    return sqrtf(nearest);
}
}

Track::Track() : trackMesh(nullptr), barrierMesh(nullptr), checkpointMesh(nullptr),
    groundMesh(nullptr), skylineMesh(nullptr), lightsMesh(nullptr), muralMesh(nullptr),
    structureMesh(nullptr), totalLaps(3) {}

Track::~Track() { destroy(); }

void Track::generateDefaultTrack() {
    segments.clear();
    // Crownline Circuit: a broad eastern sweep and a tighter, scalloped west side.
    constexpr int count = 96;
    auto point = [](float a) {
        return Vector3(72.f * cosf(a) + 12.f * cosf(2.f * a) - 8.f,
                       0, 52.f * sinf(a) + 9.f * sinf(3.f * a));
    };
    for (int i = 0; i < count; ++i) {
        Vector3 a = point(i * 2.f * PI / count);
        Vector3 b = point((i + 1) * 2.f * PI / count);
        Vector3 delta = b - a;
        segments.push_back({(a + b) * 0.5f, 13.f, delta.length(), atan2f(delta.x, delta.z)});
    }
    buildMesh();
}

void Track::addSegment(const TrackSegment& segment) { segments.push_back(segment); }

void Track::buildMesh() {
    destroy();
    if (segments.empty()) return;
    createTrackGeometry();
    createBarriers();
}

void Track::createTrackGeometry() {
    Geometry road, markings, ground, skyline, lights, mural, structures;
    for (size_t i = 0; i < segments.size(); ++i) {
        float hw = segments[i].width * 0.5f;
        strip(road, segments, i, -hw, hw, 0);
        // Orange kerbs, ice-blue gate lighting and a plum industrial skyline.
        strip(markings, segments, i, -hw + 0.65f, -hw + 0.78f, 0.025f);
        strip(markings, segments, i, hw - 0.78f, hw - 0.65f, 0.025f);
        if (i % 2 == 0) {
            strip(markings, segments, i, -hw, -hw + 0.45f, 0.03f);
            strip(markings, segments, i, hw - 0.45f, hw, 0.03f);
        }
        if (i % 4 == 0) strip(markings, segments, i, -0.07f, 0.07f, 0.025f);
        if (i % 6 == 0) {
            Vector3 center = segments[i].center * 1.55f;
            float h = 7.f + static_cast<float>((i * 13) % 23);
            center.y = h * 0.5f - 0.1f;
            skyline.box(center, Vector3(8.f, h, 8.f));
            lights.box(center + Vector3(0, h * 0.35f, 4.05f), Vector3(6.f, 0.25f, 0.06f));
        }
        if (i % 12 == 0) {
            for (int sign : {-1, 1}) {
                Vector3 base = segments[i].center + right(segments[i]) * (sign * 8.5f);
                structures.box(base + Vector3(0, 3, 0), Vector3(0.2f, 6, 0.2f));
                lights.rotatedBox(base + Vector3(0, 6, 0), Vector3(1.8f, 0.12f, 0.3f), segments[i].rotation);
            }
        }
    }
    for (int gate = 0; gate < 4; ++gate) {
        const auto& segment = segments[gate * segments.size() / 4];
        Vector3 across = right(segment);
        for (int sign : {-1, 1}) {
            Vector3 base = segment.center + across * (sign * 8.5f);
            structures.rotatedBox(base + Vector3(0, 4, 0), Vector3(1.1f, 8, 1.5f), segment.rotation);
            lights.rotatedBox(base + Vector3(0, 4.2f, 0), Vector3(1.2f, 5.5f, 0.22f), segment.rotation);
        }
        if (gate % 2 == 0) {
            // Two illuminated bridge gantries are visible from both approach directions.
            structures.rotatedBox(segment.center + Vector3(0, 8, 0), Vector3(19, 1.4f, 4), segment.rotation);
            lights.rotatedBox(segment.center + Vector3(0, 7.27f, 0), Vector3(16, 0.12f, 3.1f), segment.rotation);
        }
        const auto& board = segments[(gate * segments.size() / 4 + 6) % segments.size()];
        Vector3 origin = board.center + right(board) * 9.f + Vector3(0, 2.7f, 0);
        structures.rotatedBox(origin, Vector3(0.3f, 3.6f, 8), board.rotation);
        // Original pixel-like chevron murals on the inside face of each trackside board.
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 9; ++col) {
                if ((col + row) % 4 > 1) continue;
                Vector3 p = origin - right(board) * 0.18f + forward(board) * ((col - 4) * 0.75f);
                p.y += (row - 2) * 0.55f;
                mural.rotatedBox(p, Vector3(0.04f, 0.45f, 0.62f), board.rotation);
            }
        }
    }
    // Foundry Spire is a distinctive infield landmark, away from the drivable road.
    structures.box(Vector3(-8, 18, 0), Vector3(11, 36, 11));
    structures.box(Vector3(-8, 39, 0), Vector3(5, 6, 5));
    for (int level = 0; level < 5; ++level)
        lights.box(Vector3(-8, 8.f + level * 6.f, 0), Vector3(11.2f, 0.2f, 11.2f));
    // A chequered starting grid across the road, using asphalt as the dark squares.
    const auto& start = segments[0];
    Vector3 along = forward(start), across = right(start);
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 12; ++col) {
            if ((row + col) % 2 != 0) continue;
            float x = -6.f + col;
            Vector3 p = start.center + across * x + along * (row - 1.f) + Vector3(0, 0.04f, 0);
            markings.quad(p, p + along, p + along + across, p + across);
        }
    }
    ground.quad(Vector3(-250, -0.12f, -250), Vector3(-250, -0.12f, 250),
                Vector3(250, -0.12f, 250), Vector3(250, -0.12f, -250));
    trackMesh = road.upload();
    checkpointMesh = markings.upload();
    groundMesh = ground.upload();
    skylineMesh = skyline.upload();
    lightsMesh = lights.upload();
    muralMesh = mural.upload();
    structureMesh = structures.upload();
}

void Track::createBarriers() {
    Geometry walls;
    for (size_t i = 0; i < segments.size(); ++i) {
        size_t next = (i + 1) % segments.size();
        float hw = segments[i].width * 0.5f + 0.25f;
        for (int side : {-1, 1}) {
            Vector3 a = joint(segments, i, side * hw, 0);
            Vector3 b = joint(segments, next, side * hw, 0);
            Vector3 up(0, 1.1f, 0);
            // Two faces keep barriers visible from both trackside and menu cameras.
            walls.quad(a, b, b + up, a + up);
            walls.quad(b, a, a + up, b + up);
        }
    }
    barrierMesh = walls.upload();
}

void Track::draw(const Shader& shader) const {
    Matrix4 identity;
    shader.setMat4("uModelMatrix", identity.data());
    auto draw = [&shader](Mesh* mesh, const Vector3& color) {
        if (!mesh) return;
        shader.setVec3("uObjectColor", color.x, color.y, color.z);
        mesh->draw();
    };
    draw(groundMesh, Vector3(0.07f, 0.035f, 0.09f));
    draw(skylineMesh, Vector3(0.18f, 0.09f, 0.21f));
    draw(trackMesh, Vector3(0.12f, 0.08f, 0.15f));
    draw(barrierMesh, Vector3(0.30f, 0.13f, 0.22f));
    draw(structureMesh, Vector3(0.20f, 0.13f, 0.23f));
    draw(checkpointMesh, Vector3(1.0f, 0.30f, 0.12f));
    draw(muralMesh, Vector3(1.0f, 0.24f, 0.075f));
    shader.setFloat("uEmissive", 0.85f);
    draw(lightsMesh, Vector3(0.55f, 0.91f, 1.0f));
    shader.setFloat("uEmissive", 0);
}

void Track::destroy() {
    Mesh** meshes[] = {&trackMesh, &barrierMesh, &checkpointMesh, &groundMesh, &skylineMesh,
                      &lightsMesh, &muralMesh, &structureMesh};
    for (Mesh** mesh : meshes) {
        delete *mesh;
        *mesh = nullptr;
    }
}

bool Track::checkCollision(const Vector3& position, float radius) const {
    if (segments.empty()) return false;
    float distance = roadDistance(segments, position);
    float hw = segments[0].width * 0.5f;
    return distance + radius > hw && distance < hw + 4.f;
}

bool Track::isOnTrack(const Vector3& position) const {
    return !segments.empty() && roadDistance(segments, position) < segments[0].width * 0.5f;
}

int Track::getCheckpointIndex(const Vector3& position) const {
    if (segments.size() < 4) return -1;
    // Four ordered gates prevent the start line alone from counting as a lap.
    for (int gate = 0; gate < 4; ++gate) {
        const auto& segment = segments[gate * segments.size() / 4];
        Vector3 delta = position - segment.center;
        if (fabsf(Vector3::dot(delta, right(segment))) < segment.width * 0.5f &&
            fabsf(Vector3::dot(delta, forward(segment))) < segment.length * 0.5f + 1.f) {
            return gate;
        }
    }
    return -1;
}

int Track::crossedCheckpoint(const Vector3& from, const Vector3& to) const {
    if (segments.size() < 4) return -1;
    for (int gate = 0; gate < 4; ++gate) {
        const auto& s = segments[gate * segments.size() / 4];
        Vector3 tangent = forward(s);
        float before = Vector3::dot(from - s.center, tangent);
        float after = Vector3::dot(to - s.center, tangent);
        // Forward crossings only; interpolation catches fast frames that skip a thin gate.
        if (before < 0 && after >= 0) {
            float t = -before / (after - before);
            Vector3 intersection = from + (to - from) * t;
            if (fabsf(Vector3::dot(intersection - s.center, right(s))) < s.width * 0.5f) return gate;
        }
    }
    return -1;
}

float Track::headingAt(const Vector3& position) const {
    float nearest = std::numeric_limits<float>::max(), heading = 0;
    for (const auto& segment : segments) {
        Vector3 tangent = forward(segment), delta = position - segment.center;
        float along = std::clamp(Vector3::dot(delta, tangent), -segment.length * 0.5f, segment.length * 0.5f);
        Vector3 offset = delta - tangent * along;
        float distance = offset.x * offset.x + offset.z * offset.z;
        if (distance < nearest) { nearest = distance; heading = segment.rotation; }
    }
    return heading;
}

Vector3 Track::getStartPosition() const {
    return segments.empty() ? Vector3(0, 0.5f, 0) : segments[0].center + Vector3(0, 0.5f, 0);
}

float Track::getStartRotation() const { return segments.empty() ? 0 : segments[0].rotation; }

} // namespace RacingEngine
