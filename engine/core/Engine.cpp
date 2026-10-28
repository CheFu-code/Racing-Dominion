#include "Engine.h"
#include "AssetLoader.h"
#include "ProceduralGeometry.h"
#include <cmath>
#include <algorithm>

namespace RacingEngine {
Engine::Engine() = default;
Engine::~Engine() { shutdown(); }

bool Engine::initialize(AAssetManager* assets) {
    AssetLoader::initialize(assets);
    if (!renderer.initialize()) return false;
    track.generateDefaultTrack();
    car.initialize();
    Geometry quad;
    quad.quad(Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, -0.5f, 0),
              Vector3(0.5f, 0.5f, 0), Vector3(-0.5f, 0.5f, 0));
    smokeQuad = quad.upload();
    // Course-length-based targets are the same in the HUD and scoring rules.
    const auto& segments = track.getSegments();
    for (int sector = 0; sector < 4; ++sector) {
        float length = 0;
        for (size_t i = sector * segments.size() / 4; i < (sector + 1) * segments.size() / 4; ++i)
            length += segments[i].length;
        challenge.goldTargets[sector] = length / 20.f + 2.f + (sector == 0 ? 1.5f : 0.f);
    }
    vehicle.setPosition(track.getStartPosition());
    vehicle.setHeading(track.getStartRotation());
    car.setPosition(vehicle.getPosition());
    car.setRotation(vehicle.getHeading(), Vector3(0, 1, 0));
    renderer.setClearColor(0.14f, 0.065f, 0.185f, 1);
    initialized = true;
    return true;
}

void Engine::resize(int width, int height) {
    screenWidth = width; screenHeight = height;
    renderer.resize(width, height);
}

void Engine::setControls(float steer, float gas, float stopping) {
    steering = std::isfinite(steer) ? std::clamp(steer, -1.f, 1.f) : 0;
    throttle = std::isfinite(gas) ? std::clamp(gas, 0.f, 1.f) : 0;
    brake = std::isfinite(stopping) ? std::clamp(stopping, 0.f, 1.f) : 0;
}

void Engine::action(int code) {
    if (code == 0 && (state == GameState::MENU || (state == GameState::FINISHED && finishHold >= 0.75f))) {
        startRace();
    } else if (state == GameState::PAUSED && code != 2) {
        state = beforePause;
    } else if ((code == 1 || code == 2) && (state == GameState::RACING || state == GameState::COUNTDOWN)) {
        beforePause = state;
        state = GameState::PAUSED;
        setControls(0, 0, 0);
    }
}

void Engine::startRace() {
    challenge.reset();
    smoke.reset();
    vehicle = Vehicle();
    vehicle.setPosition(track.getStartPosition());
    vehicle.setHeading(track.getStartRotation());
    car.setPosition(vehicle.getPosition());
    car.setRotation(vehicle.getHeading(), Vector3(0, 1, 0));
    countdown = 3; skid = collisionCooldown = finishHold = 0; collisions = 0;
    setControls(0, 0, 0);
    state = GameState::COUNTDOWN;
    updateCamera(0, true);
}

void Engine::update(float dt) {
    if (!initialized || !std::isfinite(dt) || dt <= 0) return;
    dt = std::min(dt, 0.05f);
    gameTime += dt;
    if (state == GameState::MENU) {
        float angle = gameTime * 0.12f;
        renderer.getCamera().lookAt(Vector3(cosf(angle) * 115, 32, sinf(angle) * 100),
                                     Vector3(-8, 5, 0), Vector3(0, 1, 0));
    } else if (state == GameState::COUNTDOWN) {
        countdown = std::max(0.f, countdown - dt);
        if (countdown == 0) state = GameState::RACING;
        updateCamera(dt);
    } else if (state == GameState::RACING) {
        updateRacing(dt);
    } else if (state == GameState::FINISHED) {
        finishHold += dt;
        smoke.update(vehicle.getPosition(), vehicle.getHeading(), 0, dt);
        updateCamera(dt);
    }
}

void Engine::updateRacing(float dt) {
    Vector3 previous = vehicle.getPosition();
    vehicle.applySteering(steering * 0.65f, dt);
    vehicle.applyAcceleration(throttle * (1 - brake));
    vehicle.applyBrake(brake);
    vehicle.update(dt);
    collisionCooldown = std::max(0.f, collisionCooldown - dt);
    if (!track.isOnTrack(vehicle.getPosition()) || track.checkCollision(vehicle.getPosition(), 0.9f)) {
        vehicle.hitBarrier(previous, track.headingAt(previous));
        if (collisionCooldown == 0) { ++collisions; collisionCooldown = 0.6f; }
    }
    challenge.tick(dt);
    int gate = track.crossedCheckpoint(previous, vehicle.getPosition());
    if (challenge.cross(gate) && challenge.finished()) {
        state = GameState::FINISHED;
        setControls(0, 0, 0);
    }
    skid = vehicle.getSpeed() > 9 ? std::max(brake, std::abs(steering) * vehicle.getSpeed() / 40.f) : 0;
    skid = std::clamp(skid, 0.f, 1.f);
    smoke.update(vehicle.getPosition(), vehicle.getHeading(), skid > 0.4f ? skid : 0, dt);
    car.setPosition(vehicle.getPosition());
    car.setRotation(vehicle.getHeading(), Vector3(0, 1, 0));
    updateCamera(dt);
}

void Engine::updateCamera(float dt, bool snap) {
    float speed = vehicle.getSpeed();
    Vector3 forward(sinf(vehicle.getHeading()), 0, cosf(vehicle.getHeading()));
    // A modest speed pullback and forward look keep corners visible, without shake.
    renderer.getCamera().followTarget(vehicle.getPosition() + forward * (speed * 0.07f), forward,
        10.5f + speed * 0.14f, 5.f + speed * 0.025f, snap ? 1.f : 1.f - expf(-7.f * dt));
}

void Engine::drawSmoke() {
    const auto& shader = renderer.getShader();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    shader.setInt("uSoftParticle", 1);
    shader.setVec3("uObjectColor", 0.45f, 0.35f, 0.48f);
    Vector3 eye = renderer.getCamera().getPosition();
    // At most 32 short-lived billboards; reuse one mesh for every puff.
    for (const auto& p : smoke.puffs) {
        if (p.age >= 1.1f) continue;
        float yaw = atan2f(eye.x - p.position.x, eye.z - p.position.z);
        Matrix4 model = Matrix4::translate(p.position) * Matrix4::rotate(yaw, 0, 1, 0)
                      * Matrix4::scale(p.size, p.size, p.size);
        shader.setMat4("uModelMatrix", model.data());
        shader.setFloat("uOpacity", 0.24f * (1.f - p.age / 1.1f));
        smokeQuad->draw();
    }
    shader.setFloat("uOpacity", 1);
    shader.setInt("uSoftParticle", 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Engine::render() {
    if (!initialized) return;
    renderer.beginFrame();
    track.draw(renderer.getShader());
    if (state != GameState::MENU) {
        car.draw(renderer.getShader(), brake > 0.1f);
        drawSmoke();
    }
    renderer.endFrame();
}

void Engine::writeTelemetry(float* v) const {
    v[0] = vehicle.getSpeed(); v[1] = challenge.completed / 4; v[2] = challenge.raceTime;
    v[3] = static_cast<int>(state); v[4] = ceilf(countdown); v[5] = challenge.completed;
    v[6] = challenge.sectorTime(); v[7] = challenge.target(3); v[8] = challenge.target(2); v[9] = challenge.target(1);
    v[10] = challenge.lastSectorTime; v[11] = challenge.lastMedal; v[12] = challenge.dominionRank();
    v[13] = challenge.points(); v[14] = challenge.claimed(); v[15] = challenge.bestLap;
    v[16] = skid; v[17] = collisions; v[18] = challenge.sectorTime();
    v[19] = steering; v[20] = throttle; v[21] = brake;
    for (int i = 0; i < CircuitChallenge::SECTORS; ++i) v[22 + i] = challenge.medals[i];
}

void Engine::shutdown() {
    delete smokeQuad; smokeQuad = nullptr;
    if (initialized) {
        track.destroy(); car.destroy(); renderer.shutdown(); initialized = false;
    }
}
} // namespace RacingEngine
