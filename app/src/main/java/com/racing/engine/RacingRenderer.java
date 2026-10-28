package co.za.chefu.racingdominion;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicBoolean;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class RacingRenderer implements GLSurfaceView.Renderer {
    private final AssetManager assets;
    private final TouchControls controls = new TouchControls(); // UI thread only
    private final AtomicInteger action = new AtomicInteger(-1);
    private final AtomicBoolean resetClock = new AtomicBoolean(true);
    private volatile float[] input = new float[3];
    private volatile RaceSnapshot snapshot = new RaceSnapshot(new float[34]);
    private final float[] telemetry = new float[34];
    private long lastFrame;

    public RacingRenderer(AssetManager assets) { this.assets = assets; }
    @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit(assets);
        resetClock.set(true);
    }
    @Override public void onSurfaceChanged(GL10 gl, int w, int h) { nativeResize(w, h); }
    @Override public void onDrawFrame(GL10 gl) {
        long now = System.nanoTime();
        float dt = resetClock.getAndSet(false) ? 1f / 60 : Math.min(0.25f, (now - lastFrame) / 1_000_000_000f);
        lastFrame = now;
        int request = action.getAndSet(-1);
        if (request >= 0) nativeAction(request);
        float[] axes = input;
        nativeSetControls(axes[0], axes[1], axes[2]);
        while (dt > 0) {
            float step = Math.min(dt, 1f / 60);
            nativeUpdate(step);
            dt -= step;
        }
        nativeRender();
        nativeReadTelemetry(telemetry);
        snapshot = new RaceSnapshot(telemetry);
    }
    public boolean handleTouchEvent(MotionEvent event, int width, int height) {
        int type = event.getActionMasked();
        if (type == MotionEvent.ACTION_CANCEL) { clearControls(); return true; }
        if (type == MotionEvent.ACTION_DOWN &&
                (snapshot.state == RaceSnapshot.MENU || snapshot.state == RaceSnapshot.FINISHED || snapshot.state == RaceSnapshot.PAUSED)) {
            clearControls(); action.set(0); return true;
        }
        controls.resize(width, height);
        int index = event.getActionIndex();
        if (type == MotionEvent.ACTION_DOWN || type == MotionEvent.ACTION_POINTER_DOWN)
            controls.down(event.getPointerId(index), event.getX(index), event.getY(index));
        if (type == MotionEvent.ACTION_MOVE) {
            for (int i = 0; i < event.getPointerCount(); i++)
                controls.move(event.getPointerId(i), event.getX(i), event.getY(i));
        }
        if (type == MotionEvent.ACTION_UP || type == MotionEvent.ACTION_POINTER_UP)
            controls.up(event.getPointerId(index));
        input = controls.values();
        return true;
    }
    public void clearControls() { controls.clear(); input = new float[3]; }
    public void togglePause() { clearControls(); action.set(1); }
    public void background() { clearControls(); action.set(2); resetClock.set(true); }
    public void resetClock() { resetClock.set(true); }
    public RaceSnapshot snapshot() { return snapshot; }

    private native void nativeInit(AssetManager assets);
    private native void nativeResize(int w, int h);
    private native void nativeUpdate(float dt);
    private native void nativeRender();
    private native void nativeSetControls(float steer, float gas, float brake);
    private native void nativeAction(int action);
    private native void nativeReadTelemetry(float[] output);
}
