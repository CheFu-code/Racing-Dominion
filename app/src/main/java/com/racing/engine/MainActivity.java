package co.za.chefu.racingdominion;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.TextView;

public class MainActivity extends Activity {
    private GLSurfaceView glSurfaceView;
    private RacingRenderer renderer;

    private TextView speedText;
    private TextView lapText;
    private TextView timeText;
    private TextView statusText;
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
        int padding = 24;

        // Speed display (bottom-right)
        speedText = new TextView(this);
        speedText.setTextSize(32);
        speedText.setTextColor(0xFFFFFFFF);
        speedText.setShadowLayer(4, 2, 2, 0xFF000000);
        speedText.setPadding(padding, padding, padding, padding);
        FrameLayout.LayoutParams speedParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        speedParams.gravity = android.view.Gravity.BOTTOM | android.view.Gravity.END;
        layout.addView(speedText, speedParams);

        // Lap display (top-left)
        lapText = new TextView(this);
        lapText.setTextSize(24);
        lapText.setTextColor(0xFFFFFFFF);
        lapText.setShadowLayer(4, 2, 2, 0xFF000000);
        lapText.setPadding(padding, padding, padding, padding);
        FrameLayout.LayoutParams lapParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        lapParams.gravity = android.view.Gravity.TOP | android.view.Gravity.START;
        layout.addView(lapText, lapParams);

        // Time display (top-right)
        timeText = new TextView(this);
        timeText.setTextSize(24);
        timeText.setTextColor(0xFFFFFFFF);
        timeText.setShadowLayer(4, 2, 2, 0xFF000000);
        timeText.setPadding(padding, padding, padding, padding);
        FrameLayout.LayoutParams timeParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        timeParams.gravity = android.view.Gravity.TOP | android.view.Gravity.END;
        layout.addView(timeText, timeParams);

        // Status text (center)
        statusText = new TextView(this);
        statusText.setTextSize(48);
        statusText.setTextColor(0xFFFFFF00);
        statusText.setShadowLayer(6, 3, 3, 0xFF000000);
        statusText.setPadding(padding, padding, padding, padding);
        statusText.setTextAlignment(TextView.TEXT_ALIGNMENT_CENTER);
        FrameLayout.LayoutParams statusParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        statusParams.gravity = android.view.Gravity.CENTER;
        layout.addView(statusText, statusParams);
    }

    private void updateHUD() {
        if (renderer == null) return;

        int state = renderer.getGameState();

        runOnUiThread(() -> {
            // Speed
            float speed = renderer.getCurrentSpeed();
            int speedKmh = (int)(speed * 3.6f); // Convert m/s to km/h (approximate)
            speedText.setText(speedKmh + " km/h");

            // Lap
            int lap = renderer.getCurrentLap() + 1;
            int totalLaps = 3;
            lapText.setText("Lap " + lap + "/" + totalLaps);

            // Time
            float raceTime = renderer.getRaceTime();
            int minutes = (int)(raceTime / 60);
            float seconds = raceTime % 60;
            timeText.setText(String.format("%d:%05.2f", minutes, seconds));

            // Status messages
            switch (state) {
                case 0: // MENU
                    statusText.setText("TAP TO START");
                    statusText.setVisibility(TextView.VISIBLE);
                    break;
                case 2: // RACING
                    statusText.setVisibility(TextView.GONE);
                    break;
                case 3: // PAUSED
                    statusText.setText("PAUSED");
                    statusText.setVisibility(TextView.VISIBLE);
                    break;
                case 4: // FINISHED
                    statusText.setText("FINISH!\n" + String.format("%d:%05.2f", minutes, seconds));
                    statusText.setVisibility(TextView.VISIBLE);
                    break;
                default:
                    statusText.setVisibility(TextView.GONE);
                    break;
            }
        });
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
        uiHandler.post(hudUpdater);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        uiHandler.removeCallbacks(hudUpdater);
    }
}
