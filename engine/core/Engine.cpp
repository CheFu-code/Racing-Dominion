#include "Engine.h"
#include "AssetLoader.h"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "RacingEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace RacingEngine {

Engine::Engine()
    : screenWidth(0), screenHeight(0), assetManager(nullptr),
      isInitialized(false), gameTime(0.0f),
      gameState(GameState::MENU) {

    inputState = {};
    raceState = {};
    raceState.totalLaps = 3;
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize(AAssetManager* assetMgr) {
    LOGI("Initializing Racing Engine...");

    assetManager = assetMgr;
    AssetLoader::initialize(assetMgr);

    if (!renderer.initialize()) {
        LOGE("Renderer initialization failed");
        return false;
    }

    // Generate default track
    track.generateDefaultTrack();

    carModel.initialize();

    // Set initial car position at track start
    vehicle.setPosition(track.getStartPosition());
    carModel.setPosition(track.getStartPosition());
    float startRot = track.getStartRotation();
    vehicle.setHeading(startRot);
    carModel.setRotation(startRot, Vector3(0, 1, 0));

    // Set up sky/clear color
    renderer.setClearColor(0.055f, 0.085f, 0.14f, 1.0f);

    gameState = GameState::MENU;
    isInitialized = true;
    LOGI("Engine initialized successfully");

    return true;
}

void Engine::resize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    renderer.resize(width, height);
    LOGI("Screen resized to: %dx%d", width, height);
}

void Engine::update(float deltaTime) {
    if (!isInitialized) return;

    // Clamp delta time to avoid physics explosion
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    gameTime += deltaTime;

    switch (gameState) {
        case GameState::MENU:
            updateMenu(deltaTime);
            break;
        case GameState::RACING:
            updateRacing(deltaTime);
            break;
        case GameState::PAUSED:
            // No updates when paused
            break;
        case GameState::FINISHED:
            // Allow camera movement but not vehicle
            updateCamera();
            break;
        default:
            break;
    }
}

void Engine::updateMenu(float deltaTime) {
    // Slowly rotate camera around track for menu view
    float angle = gameTime * 0.3f;
    float camX = cosf(angle) * 120.0f;
    float camZ = sinf(angle) * 120.0f;
    renderer.getCamera().lookAt(
        Vector3(camX, 40, camZ),
        Vector3(0, 0, 0),
        Vector3(0, 1, 0));
}

void Engine::updateRacing(float deltaTime) {
    handleInput(deltaTime);
    vehicle.update(deltaTime);

    raceState.raceTime += deltaTime;
    raceState.currentLapTime += deltaTime;

    Vector3 pos = vehicle.getPosition();

    // Boundary check - keep car on/near track
    if (pos.y < -1.0f) {
        vehicle.setPosition(Vector3(pos.x, 0.5f, pos.z));
    }

    // Check collision with barriers
    if (track.checkCollision(pos, 1.5f)) {
        // Push back - simple collision response
        Vector3 vel = vehicle.getVelocity();
        vehicle.applyBrake(1.0f);
    }

    // Check checkpoints
    checkCheckpoints();

    // Update car model position
    carModel.setPosition(vehicle.getPosition());

    // Update car rotation based on steering
    Vector3 vel = vehicle.getVelocity();
    float speed = vehicle.getSpeed();
    if (speed > 0.5f) {
        float angle = atan2f(vel.x, vel.z);
        carModel.setRotation(angle, Vector3(0, 1, 0));
    }

    updateCamera();
}

void Engine::handleInput(float deltaTime) {
    if (!inputState.touching) {
        inputState.accelerationInput = 0;
        inputState.brakeInput = 0;
        inputState.steeringInput *= 0.9f; // Decay steering
        vehicle.applySteering(inputState.steeringInput * 0.8f, deltaTime);
        return;
    }

    // Touch-based controls:
    // Left half of screen = steering left/right (based on Y position)
    // Right half of screen = accelerate (upper) or brake (lower)
    float normalizedX = inputState.touchX / (float)(screenWidth > 0 ? screenWidth : 1);
    float normalizedY = inputState.touchY / (float)(screenHeight > 0 ? screenHeight : 1);

    if (normalizedX < 0.5f) {
        // Left side - steering based on vertical position
        // Top of left side = steer left, bottom = steer right
        inputState.steeringInput = (0.5f - normalizedY) * 2.0f;
        inputState.steeringInput = fmaxf(-1.0f, fminf(1.0f, inputState.steeringInput));
    } else {
        // Right side - accelerate (top) or brake (bottom)
        if (normalizedY < 0.5f) {
            inputState.accelerationInput = (0.5f - normalizedY) * 2.0f;
            inputState.brakeInput = 0;
        } else {
            inputState.accelerationInput = 0;
            inputState.brakeInput = (normalizedY - 0.5f) * 2.0f;
        }
    }

    vehicle.applySteering(inputState.steeringInput * 0.8f, deltaTime);

    if (inputState.accelerationInput > 0.01f) {
        vehicle.applyAcceleration(inputState.accelerationInput);
    }
    if (inputState.brakeInput > 0.01f) {
        vehicle.applyBrake(inputState.brakeInput);
    }
}

void Engine::updateCamera() {
    Vector3 carPos = vehicle.getPosition();
    Vector3 carVel = vehicle.getVelocity();
    float speed = vehicle.getSpeed();

    Vector3 forward;
    if (speed > 0.5f) {
        forward = carVel.normalized();
    } else {
        forward = Vector3(sinf(vehicle.getHeading()), 0, cosf(vehicle.getHeading()));
    }

    renderer.getCamera().followTarget(carPos, forward, 12.0f, 6.0f, 0.05f);
}

void Engine::checkCheckpoints() {
    Vector3 pos = vehicle.getPosition();
    int cpIdx = track.getCheckpointIndex(pos);

    if (cpIdx >= 0 && cpIdx == (raceState.lastCheckpoint + 1) % 4) {
        if (cpIdx == 0) {
            // Completed a lap
            raceState.currentLap++;
            if (raceState.currentLapTime < raceState.bestLapTime || raceState.bestLapTime == 0) {
                raceState.bestLapTime = raceState.currentLapTime;
            }
            raceState.currentLapTime = 0;

            if (raceState.currentLap >= raceState.totalLaps) {
                raceState.finished = true;
                gameState = GameState::FINISHED;
                LOGI("Race finished! Total time: %.2f", raceState.raceTime);
            }
        }
        raceState.lastCheckpoint = cpIdx;
    }
}

void Engine::render() {
    if (!isInitialized) return;

    renderer.beginFrame();

    // Draw track
    track.draw(renderer.getShader());

    // Draw car
    if (gameState != GameState::MENU) {
        carModel.draw(renderer.getShader());
    }

    renderer.endFrame();
}

void Engine::shutdown() {
    if (isInitialized) {
        LOGI("Shutting down engine...");
        track.destroy();
        carModel.destroy();
        renderer.shutdown();
        isInitialized = false;
    }
}

void Engine::onTouchDown(float x, float y) {
    inputState.touching = true;
    inputState.touchX = x;
    inputState.touchY = y;
    inputState.touchStartX = x;
    inputState.touchStartY = y;

    if (gameState == GameState::MENU) {
        startRace();
    } else if (gameState == GameState::FINISHED) {
        resetRace();
        startRace();
    }
}

void Engine::onTouchMove(float x, float y) {
    inputState.touchX = x;
    inputState.touchY = y;
}

void Engine::onTouchUp(float x, float y) {
    inputState.touching = false;
    inputState.accelerationInput = 0;
    inputState.brakeInput = 0;
}

void Engine::startRace() {
    LOGI("Starting race...");
    resetRace();
    gameState = GameState::RACING;
}

void Engine::pauseGame() {
    if (gameState == GameState::RACING) {
        gameState = GameState::PAUSED;
    }
}

void Engine::resumeGame() {
    if (gameState == GameState::PAUSED) {
        gameState = GameState::RACING;
    }
}

void Engine::resetRace() {
    vehicle = Vehicle();
    vehicle.setPosition(track.getStartPosition());
    carModel.setPosition(track.getStartPosition());
    carModel.setRotation(track.getStartRotation(), Vector3(0, 1, 0));
    vehicle.setHeading(track.getStartRotation());

    raceState = {};
    raceState.totalLaps = 3;
}

} // namespace RacingEngine
