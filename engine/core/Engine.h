#ifndef ENGINE_H
#define ENGINE_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/asset_manager.h>
#include "Renderer.h"
#include "Track.h"
#include "Vehicle.h"
#include "DominionCar.h"
#include "CircuitChallenge.h"
#include "SmokeTrail.h"

namespace RacingEngine {
// Numeric values are shared with RaceSnapshot.java.
enum class GameState { MENU = 0, LOADING = 1, RACING = 2, PAUSED = 3, FINISHED = 4, COUNTDOWN = 5 };

class Engine {
public:
    Engine();
    ~Engine();
    bool initialize(AAssetManager* assets);
    void resize(int width, int height);
    void update(float dt);
    void render();
    void shutdown();
    void setControls(float steer, float throttle, float brake);
    void action(int action); // 0 activate/resume, 1 pause toggle, 2 background pause
    void writeTelemetry(float* values) const; // 34 entries; see RaceSnapshot.

private:
    int screenWidth = 0, screenHeight = 0;
    bool initialized = false;
    float gameTime = 0, countdown = 3, collisionCooldown = 0, skid = 0, finishHold = 0;
    float steering = 0, throttle = 0, brake = 0;
    int collisions = 0;
    GameState state = GameState::MENU, beforePause = GameState::RACING;
    Renderer renderer;
    Vehicle vehicle;
    Track track;
    DominionCar car;
    CircuitChallenge challenge;
    SmokeTrail smoke;
    Mesh* smokeQuad = nullptr;

    void startRace();
    void updateCamera(float dt, bool snap = false);
    void updateRacing(float dt);
    void drawSmoke();
};
} // namespace RacingEngine
#endif
