package co.za.chefu.racingdominion;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;
import java.util.HashSet;
import java.util.Set;

/** SoundPool effects are original synthesized samples, controlled on the main thread. */
public class AudioHelper {
    private SoundPool pool;
    private final AudioManager manager;
    private final Set<Integer> ready = new HashSet<>();
    private final int engineId, tireId, impactId, countId, goId, sectorId, finishId;
    private final int[] shots = new int[8];
    private int engineStream, tireStream, nextShot;
    private int previousState = -1, previousCount = -1, previousSectors, previousCollisions;
    private boolean enabled = true, focused;
    private final AudioManager.OnAudioFocusChangeListener focusListener = change -> {
        focused = change == AudioManager.AUDIOFOCUS_GAIN;
        if (!focused) stopAll();
    };

    public AudioHelper(Context context) {
        manager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        pool = new SoundPool.Builder().setMaxStreams(8).setAudioAttributes(
                new AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION).build()).build();
        pool.setOnLoadCompleteListener((source, id, status) -> {
            if (source == pool && status == 0) ready.add(id);
        });
        engineId = pool.load(context, R.raw.rd_engine, 1);
        tireId = pool.load(context, R.raw.rd_tire, 1);
        impactId = pool.load(context, R.raw.rd_impact, 1);
        countId = pool.load(context, R.raw.rd_count, 1);
        goId = pool.load(context, R.raw.rd_go, 1);
        sectorId = pool.load(context, R.raw.rd_sector, 1);
        finishId = pool.load(context, R.raw.rd_finish, 1);
    }
    public void resume() {
        if (enabled && manager != null)
            focused = manager.requestAudioFocus(focusListener, AudioManager.STREAM_MUSIC,
                    AudioManager.AUDIOFOCUS_GAIN) == AudioManager.AUDIOFOCUS_REQUEST_GRANTED;
    }
    public void suspend() {
        stopAll(); focused = false;
        if (manager != null) manager.abandonAudioFocus(focusListener);
    }
    public void setEnabled(boolean value) {
        enabled = value;
        if (enabled) resume(); else suspend();
    }
    public boolean isEnabled() { return enabled; }
    private int play(int sound, float volume, int loop, float rate) {
        if (!enabled || !focused || pool == null || !ready.contains(sound)) return 0;
        return pool.play(sound, volume, volume, loop == -1 ? 0 : 1, loop, rate);
    }
    private void cue(int id) { shots[nextShot++ % shots.length] = play(id, 0.55f, 0, 1); }
    public void update(RaceSnapshot race) {
        if (pool == null) return;
        if (enabled && focused) {
            if (race.state == RaceSnapshot.COUNTDOWN &&
                    (race.countdown != previousCount || previousState != RaceSnapshot.COUNTDOWN)) cue(countId);
            if (race.state == RaceSnapshot.RACING && previousState == RaceSnapshot.COUNTDOWN) cue(goId);
            if (race.state == RaceSnapshot.FINISHED && previousState != RaceSnapshot.FINISHED) cue(finishId);
            if (race.completed > previousSectors && race.state != RaceSnapshot.FINISHED) cue(sectorId);
            if (race.collisions > previousCollisions) cue(impactId);
            if (race.state == RaceSnapshot.RACING) {
                updateEngineSound(race.speed);
                if (race.skid > 0.4f) {
                    if (tireStream == 0) tireStream = play(tireId, 0.2f, -1, 1);
                    if (tireStream != 0) pool.setVolume(tireStream, race.skid * 0.25f, race.skid * 0.25f);
                } else if (tireStream != 0) { pool.stop(tireStream); tireStream = 0; }
            } else if (race.state == RaceSnapshot.PAUSED) stopAll(); else stopLoops();
        }
        previousState = race.state; previousCount = race.countdown;
        previousSectors = race.completed; previousCollisions = race.collisions;
    }
    public void updateEngineSound(float speed) {
        float rate = Math.max(0.65f, Math.min(2, 0.65f + speed / 45));
        float volume = Math.min(0.38f, 0.13f + speed / 200);
        if (engineStream == 0) engineStream = play(engineId, volume, -1, rate);
        if (engineStream != 0 && pool != null) {
            pool.setRate(engineStream, rate); pool.setVolume(engineStream, volume, volume);
        }
    }
    public void stopEngineSound() {
        if (pool != null && engineStream != 0) pool.stop(engineStream);
        engineStream = 0;
    }
    // Retained for the optional native audio bridge.
    public void playSound(int type) { cue(type == 1 ? tireId : type == 2 ? impactId : type == 6 ? finishId : countId); }
    private void stopLoops() {
        stopEngineSound();
        if (pool != null && tireStream != 0) pool.stop(tireStream);
        tireStream = 0;
    }
    private void stopAll() {
        stopLoops();
        if (pool != null) for (int id : shots) if (id != 0) pool.stop(id);
    }
    public void release() {
        suspend();
        if (pool != null) { pool.release(); pool = null; }
        ready.clear();
    }
}
