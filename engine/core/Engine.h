#ifndef ENGINE_H
#define ENGINE_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/asset_manager.h>
#include "Renderer.h"
#include "Camera.h"
#include "Track.h"
#include "Vehicle.h"
#include "Mesh.h"
#include "DominionCar.h"

namespace RacingEngine {

enum class GameState {
    MENU,
    LOADING,
    RACING,
    PAUSED,
    FINISHED
};

struct InputState {
    bool touching;
    float touchX, touchY;
    float touchStartX, touchStartY;
    float steeringInput;   // -1 to 1
    float accelerationInput; // 0 to 1
    float brakeInput;      // 0 to 1
    bool touchLeft;        // left side of screen for steering
    bool touchRight;       // right side of screen for steering
};

struct RaceState {
    int currentLap;
    int totalLaps;
    int lastCheckpoint;
    float raceTime;
    float bestLapTime;
    float currentLapTime;
    bool finished;
};

class Engine {
public:
    Engine();
    ~Engine();

    bool initialize(AAssetManager* assetManager);
    void resize(int width, int height);
    void update(float deltaTime);
    void render();
    void shutdown();

    // Input handling
    void onTouchDown(float x, float y);
    void onTouchMove(float x, float y);
    void onTouchUp(float x, float y);

    // Game state
    GameState getGameState() const { return gameState; }
    void startRace();
    void pauseGame();
    void resumeGame();

    // For HUD display
    float getSpeed() const { return vehicle.getSpeed(); }
    int getCurrentLap() const { return raceState.currentLap; }
    int getTotalLaps() const { return raceState.totalLaps; }
    float getRaceTime() const { return raceState.raceTime; }
    bool isRaceFinished() const { return raceState.finished; }

private:
    int screenWidth;
    int screenHeight;
    AAssetManager* assetManager;

    bool isInitialized;
    float gameTime;

    Renderer renderer;
    Vehicle vehicle;
    Track track;
    InputState inputState;
    RaceState raceState;
    GameState gameState;

    // Car model for rendering
    DominionCar carModel;

    void updateMenu(float deltaTime);
    void updateRacing(float deltaTime);
    void handleInput(float deltaTime);
    void updateCamera();
    void checkCheckpoints();
    void resetRace();
};

} // namespace RacingEngine

#endif // ENGINE_H
