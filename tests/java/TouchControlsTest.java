package co.za.chefu.racingdominion;

/** Plain Java checks: no emulator or Android test runner is required. */
public final class TouchControlsTest {
    private static void near(float expected, float actual) {
        if (Math.abs(expected - actual) > 0.001f) throw new AssertionError(expected + " != " + actual);
    }
    public static void main(String[] args) {
        TouchControls controls = new TouchControls();
        controls.resize(800, 400);
        controls.down(7, 30, 300); // Left steering, positive native input.
        controls.down(42, 720, 270); // Independent gas finger.
        near(1, controls.values()[0]); near(1, controls.values()[1]); near(0, controls.values()[2]);
        controls.move(42, 720, 340); // Same finger slides to brake.
        near(1, controls.values()[0]); near(0, controls.values()[1]); near(1, controls.values()[2]);
        controls.up(7); // Steering release does not release the pedal.
        near(0, controls.values()[0]); near(1, controls.values()[2]);
        controls.up(42);
        for (float axis : controls.values()) near(0, axis);
        controls.down(1, 90, 300); near(0, controls.values()[0]);
        controls.down(2, 30, 300); near(0, controls.values()[0]); // Extra left finger cannot steal control.
        controls.move(1, 780, 260); near(-1, controls.values()[0]); // Ownership stays on steering.
        near(0, controls.values()[1]);
        controls.clear(); for (float axis : controls.values()) near(0, axis);
        controls.resize(1600, 800);
        controls.down(99, 60, 600); controls.down(3, 1450, 540);
        near(1, controls.values()[0]); near(1, controls.values()[1]);
        controls.move(3, 1450, 680); near(1, controls.values()[2]);
        controls.clear();
        controls.resize(800, 600); // Tablet: pedal boundary follows the bottom controls.
        controls.down(4, 700, 470); near(1, controls.values()[1]);
        controls.move(4, 700, 530); near(1, controls.values()[2]);
        controls.clear();
        float[] v = new float[34]; v[3] = RaceSnapshot.FINISHED; v[5] = 12; v[22] = 3;
        RaceSnapshot snapshot = new RaceSnapshot(v);
        v[22] = 0;
        if (snapshot.lap() != 3 || snapshot.medal(0) != 3) throw new AssertionError("Snapshot changed after publication");
        System.out.println("Multitouch ownership, independent releases, cancellation, scaling, and telemetry snapshots passed.");
    }
}
