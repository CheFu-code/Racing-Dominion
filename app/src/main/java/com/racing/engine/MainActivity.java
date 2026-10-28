package co.za.chefu.racingdominion;

import android.app.Activity;
import android.content.SharedPreferences;
import android.graphics.drawable.GradientDrawable;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.TypedValue;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;

public class MainActivity extends Activity {
    private GLSurfaceView surface;
    private RacingRenderer renderer;
    private RacingHudView hud;
    private AudioHelper audio;
    private SharedPreferences preferences;
    private Button pauseButton, soundButton;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private float bestTime;
    private int bestRank, previousState = -1;
    private boolean personalBest;
    private final Runnable updater = new Runnable() {
        @Override public void run() { updateHUD(); handler.postDelayed(this, 80); }
    };
    static { System.loadLibrary("racing_engine"); }

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        preferences = getSharedPreferences("dominion_foundry_v1", MODE_PRIVATE);
        bestTime = preferences.getFloat("best_time", 0);
        bestRank = preferences.getInt("best_rank", 0);
        audio = new AudioHelper(this);
        audio.setEnabled(preferences.getBoolean("sound_enabled", true));
        setVolumeControlStream(android.media.AudioManager.STREAM_MUSIC);

        FrameLayout layout = new FrameLayout(this);
        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setPreserveEGLContextOnPause(true);
        renderer = new RacingRenderer(getAssets());
        surface.setRenderer(renderer);
        surface.setOnTouchListener((v, event) -> renderer.handleTouchEvent(event, v.getWidth(), v.getHeight()));
        layout.addView(surface);
        hud = new RacingHudView(this);
        layout.addView(hud, new FrameLayout.LayoutParams(-1, -1));
        pauseButton = button("PAUSE");
        soundButton = button("SOUND");
        pauseButton.setOnClickListener(v -> renderer.togglePause());
        soundButton.setOnClickListener(v -> {
            audio.setEnabled(!audio.isEnabled());
            preferences.edit().putBoolean("sound_enabled", audio.isEnabled()).apply();
            updateHUD();
        });
        layout.addView(pauseButton, new FrameLayout.LayoutParams(1, 1));
        layout.addView(soundButton, new FrameLayout.LayoutParams(1, 1));
        setContentView(layout);
    }
    private Button button(String label) {
        Button button = new Button(this);
        button.setText(label); button.setAllCaps(true); button.setTextColor(0xFF8DE8FF);
        button.setPadding(0, 0, 0, 0); button.setMinHeight(0); button.setMinWidth(0);
        button.setMinimumHeight(0); button.setMinimumWidth(0);
        GradientDrawable background = new GradientDrawable();
        background.setColor(0xEF421D43); background.setCornerRadius(12);
        background.setStroke(1, 0xFF8DE8FF); button.setBackground(background);
        return button;
    }
    private void place(Button button, float x, float y, float width, float height, float scale) {
        FrameLayout.LayoutParams p = (FrameLayout.LayoutParams) button.getLayoutParams();
        int left = Math.round(x * scale), top = Math.round(y * scale);
        int w = Math.round(width * scale), h = Math.round(height * scale);
        if (p.leftMargin != left || p.topMargin != top || p.width != w || p.height != h) {
            p.leftMargin = left; p.topMargin = top; p.width = w; p.height = h; button.setLayoutParams(p);
            button.setTextSize(TypedValue.COMPLEX_UNIT_PX, 11 * scale);
        }
    }
    private void updateHUD() {
        if (renderer == null) return;
        RaceSnapshot race = renderer.snapshot();
        if (race.state == RaceSnapshot.FINISHED && previousState != RaceSnapshot.FINISHED) {
            personalBest = bestTime == 0 || race.time < bestTime;
            if (personalBest) bestTime = race.time;
            bestRank = Math.max(bestRank, race.rank);
            preferences.edit().putFloat("best_time", bestTime).putInt("best_rank", bestRank).apply();
        }
        if (race.state == RaceSnapshot.COUNTDOWN) personalBest = false;
        previousState = race.state;
        hud.update(race, bestTime, bestRank, personalBest);
        audio.update(race);
        boolean canPause = race.state == RaceSnapshot.RACING || race.state == RaceSnapshot.COUNTDOWN || race.state == RaceSnapshot.PAUSED;
        pauseButton.setVisibility(canPause ? View.VISIBLE : View.GONE);
        pauseButton.setText(race.state == RaceSnapshot.PAUSED ? "RESUME" : "PAUSE");
        pauseButton.setContentDescription(race.state == RaceSnapshot.PAUSED ? "Resume race" : "Pause race");
        soundButton.setText(audio.isEnabled() ? "SOUND" : "MUTED");
        soundButton.setContentDescription(audio.isEnabled() ? "Sound on. Tap to mute." : "Sound off. Tap to enable.");
        if (hud.getWidth() > 0 && hud.getHeight() > 0) {
            float scale = Math.min(hud.getWidth() / 800f, hud.getHeight() / 400f);
            float width = hud.getWidth() / scale;
            place(pauseButton, width - 350, 20, 64, 44, scale);
            place(soundButton, race.state == RaceSnapshot.MENU ? width - 110 : width - 278,
                    20, race.state == RaceSnapshot.MENU ? 86 : 64, 44, scale);
        }
    }
    @Override protected void onPause() {
        handler.removeCallbacks(updater);
        renderer.background(); audio.suspend(); surface.onPause();
        super.onPause();
    }
    @Override protected void onResume() {
        super.onResume();
        renderer.resetClock(); surface.onResume(); audio.resume();
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        handler.post(updater);
    }
    @Override public void onBackPressed() {
        int state = renderer.snapshot().state;
        if (state == RaceSnapshot.RACING || state == RaceSnapshot.COUNTDOWN || state == RaceSnapshot.PAUSED)
            renderer.togglePause();
        else super.onBackPressed();
    }
    @Override protected void onDestroy() {
        handler.removeCallbacks(updater); audio.release(); super.onDestroy();
    }
}
