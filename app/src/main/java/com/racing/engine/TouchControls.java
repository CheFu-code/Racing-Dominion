package co.za.chefu.racingdominion;

/** Pointer ownership stays fixed until release, even when fingers cross the screen. */
final class TouchControls {
    private int steeringPointer = -1, pedalPointer = -1;
    private float width = 800, height = 400;
    private float steering, throttle, brake;

    void resize(float width, float height) {
        this.width = Math.max(1, width);
        this.height = Math.max(1, height);
    }
    void down(int id, float x, float y) {
        if (x < width * 0.5f) {
            if (steeringPointer == -1) steeringPointer = id;
        } else if (pedalPointer == -1) pedalPointer = id;
        move(id, x, y);
    }
    void move(int id, float x, float y) {
        float scale = Math.min(width / 800f, height / 400f);
        if (id == steeringPointer) {
            float axis = (90 * scale - x) / (60 * scale);
            steering = Math.max(-1, Math.min(1, axis));
            if (Math.abs(steering) < 0.08f) steering = 0;
        }
        if (id == pedalPointer) {
            throttle = y < height - 105 * scale ? 1 : 0;
            brake = 1 - throttle;
        }
    }
    void up(int id) {
        if (id == steeringPointer) { steeringPointer = -1; steering = 0; }
        if (id == pedalPointer) { pedalPointer = -1; throttle = brake = 0; }
    }
    void clear() { steeringPointer = pedalPointer = -1; steering = throttle = brake = 0; }
    float[] values() { return new float[]{steering, throttle, brake}; }
}
