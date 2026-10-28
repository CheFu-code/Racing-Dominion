package co.za.chefu.racingdominion;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class RacingRenderer implements GLSurfaceView.Renderer {
    private AssetManager assetManager;
    private long lastFrameTime;
    private int surfaceWidth;
    private int surfaceHeight;

    // HUD data (read from native, used by UI thread)
    private volatile float currentSpeed;
    private volatile int currentLap;
    private volatile float raceTime;
    private volatile boolean raceFinished;
    private volatile int gameState;

    public RacingRenderer(AssetManager assetManager) {
        this.assetManager = assetManager;
        this.lastFrameTime = System.currentTimeMillis();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit(assetManager);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        surfaceWidth = width;
        surfaceHeight = height;
        nativeResize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        long currentTime = System.currentTimeMillis();
        float deltaTime = (currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;

        // Clamp delta time
        if (deltaTime > 0.1f) deltaTime = 0.016f;

        nativeUpdate(deltaTime);
        nativeRender();

        // Update HUD data from native
        currentSpeed = nativeGetSpeed();
        currentLap = nativeGetLap();
        raceTime = nativeGetRaceTime();
        raceFinished = nativeIsFinished();
        gameState = nativeGetGameState();
    }

    public boolean handleTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                nativeOnTouchDown(x, y);
                return true;
            case MotionEvent.ACTION_MOVE:
                nativeOnTouchMove(x, y);
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                nativeOnTouchUp(x, y);
                return true;
        }
        return false;
    }

    // HUD getters
    public float getCurrentSpeed() { return currentSpeed; }
    public int getCurrentLap() { return currentLap; }
    public float getRaceTime() { return raceTime; }
    public boolean isRaceFinished() { return raceFinished; }
    public int getGameState() { return gameState; }
    public int getSurfaceWidth() { return surfaceWidth; }
    public int getSurfaceHeight() { return surfaceHeight; }

    // Native methods
    private native void nativeInit(AssetManager assetManager);
    private native void nativeResize(int width, int height);
    private native void nativeUpdate(float deltaTime);
    private native void nativeRender();
    private native void nativeOnTouchDown(float x, float y);
    private native void nativeOnTouchMove(float x, float y);
    private native void nativeOnTouchUp(float x, float y);
    private native float nativeGetSpeed();
    private native int nativeGetLap();
    private native float nativeGetRaceTime();
    private native boolean nativeIsFinished();
    private native int nativeGetGameState();
}
