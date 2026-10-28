package co.za.chefu.racingdominion;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.SoundPool;

public class AudioHelper {
    private SoundPool soundPool;
    private int engineSoundId;
    private int brakeSoundId;
    private int collisionSoundId;
    private int countdownSoundId;
    private int finishSoundId;
    private int engineStreamId;
    private boolean enginePlaying;

    public AudioHelper(Context context) {
        AudioAttributes audioAttributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();

        soundPool = new SoundPool.Builder()
                .setMaxStreams(8)
                .setAudioAttributes(audioAttributes)
                .build();

        // Sound resources would be loaded from res/raw
        // Using 0 as placeholder since actual sound files would need to be included
        engineSoundId = 0;
        brakeSoundId = 0;
        collisionSoundId = 0;
        countdownSoundId = 0;
        finishSoundId = 0;
        enginePlaying = false;
    }

    public void playSound(int soundType) {
        if (soundPool == null) return;

        float volume = 0.5f;
        switch (soundType) {
            case 1: // Brake
                if (brakeSoundId != 0) soundPool.play(brakeSoundId, volume, volume, 1, 0, 1.0f);
                break;
            case 2: // Collision
                if (collisionSoundId != 0) soundPool.play(collisionSoundId, volume, volume, 1, 0, 1.0f);
                break;
            case 3: // Countdown beep
            case 4:
            case 5:
                if (countdownSoundId != 0) soundPool.play(countdownSoundId, volume, volume, 1, 0, 1.0f);
                break;
            case 6: // Finish
                if (finishSoundId != 0) soundPool.play(finishSoundId, volume, volume, 1, 0, 1.0f);
                break;
        }
    }

    public void updateEngineSound(float speed) {
        if (soundPool == null || engineSoundId == 0) return;

        float pitch = 0.5f + (speed / 80.0f) * 1.5f;
        pitch = Math.max(0.5f, Math.min(2.0f, pitch));
        float volume = 0.3f + (speed / 80.0f) * 0.4f;
        volume = Math.max(0.1f, Math.min(0.7f, volume));

        if (!enginePlaying) {
            engineStreamId = soundPool.play(engineSoundId, volume, volume, 0, -1, pitch);
            enginePlaying = true;
        } else {
            soundPool.setRate(engineStreamId, pitch);
            soundPool.setVolume(engineStreamId, volume, volume);
        }
    }

    public void stopEngineSound() {
        if (soundPool != null && enginePlaying) {
            soundPool.stop(engineStreamId);
            enginePlaying = false;
        }
    }

    public void release() {
        if (soundPool != null) {
            soundPool.release();
            soundPool = null;
        }
    }
}
