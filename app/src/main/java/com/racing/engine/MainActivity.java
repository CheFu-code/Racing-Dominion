package co.za.chefu.racingdominion;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

public class MainActivity extends Activity {
    private GLSurfaceView glSurfaceView;
    private RacingRenderer renderer;

    private RacingHudView hud;
    private Handler uiHandler;
    private Runnable hudUpdater;

    static {
        System.loadLibrary("racing_engine");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Fullscreen mode
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        );

        // Create layout with GLSurfaceView and HUD overlay
        FrameLayout layout = new FrameLayout(this);

        // Create OpenGL ES 3.0 context
        glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(3);

        renderer = new RacingRenderer(getAssets());
        glSurfaceView.setRenderer(renderer);

        // Touch handling
        glSurfaceView.setOnTouchListener((v, event) -> {
            return renderer.handleTouchEvent(event);
        });

        layout.addView(glSurfaceView);

        // Create HUD overlay
        setupHUD(layout);

        setContentView(layout);

        // Start HUD update loop
        uiHandler = new Handler(Looper.getMainLooper());
        hudUpdater = new Runnable() {
            @Override
            public void run() {
                updateHUD();
                uiHandler.postDelayed(this, 100); // Update 10 times per second
            }
        };
    }

    private void setupHUD(FrameLayout layout) {
        hud = new RacingHudView(this);
        layout.addView(hud, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
    }

    private void updateHUD() {
        if (renderer != null) hud.update(renderer);
    }

    @Override
    protected void onPause() {
        super.onPause();
        uiHandler.removeCallbacks(hudUpdater);
        glSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glSurfaceView.onResume();
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        uiHandler.post(hudUpdater);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        uiHandler.removeCallbacks(hudUpdater);
    }
}
