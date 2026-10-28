#include "DominionCar.h"
#include "ProceduralGeometry.h"
#include "Shader.h"

namespace RacingEngine {

void DominionCar::initialize() {
    destroy();
    Geometry body, dark, lights, tail, livery;
    body.box(Vector3(0, 0.25f, 0), Vector3(2.05f, 0.55f, 4.2f), 0.78f);
    body.box(Vector3(0, 0.48f, -1.55f), Vector3(2.12f, 0.14f, 0.8f));
    // Canopy, front splitter, rear diffuser and a floating rear wing.
    dark.box(Vector3(0, 0.76f, -0.25f), Vector3(1.5f, 0.56f, 1.75f), 0.74f);
    dark.box(Vector3(0, -0.01f, 1.75f), Vector3(1.95f, 0.13f, 0.9f), 0.85f);
    dark.box(Vector3(0, 0.02f, -1.94f), Vector3(1.9f, 0.22f, 0.36f));
    dark.box(Vector3(0, 1.0f, -1.7f), Vector3(2.35f, 0.12f, 0.43f));
    for (int side : {-1, 1}) {
        dark.box(Vector3(side * 0.65f, 0.72f, -1.7f), Vector3(0.10f, 0.5f, 0.16f));
        for (float z : {-1.22f, 1.2f}) {
            dark.wheel(Vector3(side * 1.01f, -0.08f, z), 0.42f, 0.32f);
            livery.wheel(Vector3(side * 1.19f, -0.08f, z), 0.20f, 0.025f);
        }
        lights.box(Vector3(side * 0.58f, 0.44f, 2.13f), Vector3(0.42f, 0.07f, 0.06f));
        lights.box(Vector3(side * 0.76f, 0.34f, 2.13f), Vector3(0.07f, 0.22f, 0.07f));
        body.box(Vector3(side * 0.86f, 0.46f, -0.9f), Vector3(0.5f, 0.23f, 1.4f), 0.6f);
        dark.box(Vector3(side * 1.08f, 0.32f, -0.4f), Vector3(0.03f, 0.21f, 0.65f));
        livery.box(Vector3(side * 1.06f, 0.1f, 0), Vector3(0.04f, 0.08f, 1.4f));
        lights.box(Vector3(side * 1.1f, 1.02f, -1.7f), Vector3(0.1f, 0.13f, 0.44f));
        tail.box(Vector3(side * 0.64f, 0.46f, -2.1f), Vector3(0.48f, 0.08f, 0.06f));
    }
    // A narrow longitudinal racing stripe is part of the mesh, not a sample texture.
    dark.box(Vector3(0, 0.535f, 1.32f), Vector3(0.18f, 0.02f, 1.35f));
    livery.rotatedBox(Vector3(-0.35f, 0.54f, 1.2f), Vector3(0.12f, 0.02f, 1.45f), -0.18f);
    livery.rotatedBox(Vector3(0.35f, 0.54f, 1.2f), Vector3(0.12f, 0.02f, 1.45f), -0.18f);
    parts[0].addMesh(body.upload());
    parts[1].addMesh(dark.upload());
    parts[2].addMesh(lights.upload());
    parts[3].addMesh(tail.upload());
    parts[4].addMesh(livery.upload());
}

void DominionCar::setPosition(const Vector3& position) {
    for (auto& part : parts) part.setPosition(position);
}

void DominionCar::setRotation(float angle, const Vector3& axis) {
    for (auto& part : parts) part.setRotation(angle, axis);
}

void DominionCar::draw(const Shader& shader, bool braking) const {
    const Vector3 colors[] = {{1.0f, 0.24f, 0.075f}, {0.035f, 0.018f, 0.055f},
                              {0.55f, 0.91f, 1.0f}, {1.0f, 0.05f, 0.09f}, {0.72f, 0.85f, 0.95f}};
    for (size_t i = 0; i < parts.size(); ++i) {
        shader.setFloat("uEmissive", i == 2 ? 0.8f : i == 3 && braking ? 1.f : 0.f);
        shader.setVec3("uObjectColor", colors[i].x, colors[i].y, colors[i].z);
        parts[i].draw(shader);
    }
    shader.setFloat("uEmissive", 0);
}

void DominionCar::destroy() {
    for (auto& part : parts) part.destroy();
}

} // namespace RacingEngine
