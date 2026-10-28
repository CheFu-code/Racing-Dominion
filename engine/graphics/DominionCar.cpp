#include "DominionCar.h"
#include "ProceduralGeometry.h"
#include "Shader.h"

namespace RacingEngine {

void DominionCar::initialize() {
    destroy();
    Geometry body, dark, lights, tail;
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
            dark.box(Vector3(side * 1.01f, 0.02f, z), Vector3(0.32f, 0.65f, 0.8f));
        }
        lights.box(Vector3(side * 0.58f, 0.44f, 2.06f), Vector3(0.42f, 0.07f, 0.06f));
        tail.box(Vector3(side * 0.64f, 0.46f, -2.1f), Vector3(0.48f, 0.08f, 0.06f));
    }
    // A narrow longitudinal racing stripe is part of the mesh, not a sample texture.
    dark.box(Vector3(0, 0.535f, 1.32f), Vector3(0.18f, 0.02f, 1.35f));
    parts[0].addMesh(body.upload());
    parts[1].addMesh(dark.upload());
    parts[2].addMesh(lights.upload());
    parts[3].addMesh(tail.upload());
}

void DominionCar::setPosition(const Vector3& position) {
    for (auto& part : parts) part.setPosition(position);
}

void DominionCar::setRotation(float angle, const Vector3& axis) {
    for (auto& part : parts) part.setRotation(angle, axis);
}

void DominionCar::draw(const Shader& shader) const {
    const Vector3 colors[] = {{0.70f, 0.95f, 0.18f}, {0.025f, 0.045f, 0.07f},
                              {0.65f, 0.94f, 1.0f}, {1.0f, 0.08f, 0.12f}};
    for (size_t i = 0; i < parts.size(); ++i) {
        shader.setVec3("uObjectColor", colors[i].x, colors[i].y, colors[i].z);
        parts[i].draw(shader);
    }
}

void DominionCar::destroy() {
    for (auto& part : parts) part.destroy();
}

} // namespace RacingEngine
