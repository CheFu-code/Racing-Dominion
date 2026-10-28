package co.za.chefu.racingdominion;

/** One immutable GL-thread snapshot prevents mixed-frame UI and audio reads. */
public final class RaceSnapshot {
    public static final int MENU = 0, RACING = 2, PAUSED = 3, FINISHED = 4, COUNTDOWN = 5;
    public final int state, countdown, completed, lastMedal, rank, points, claimed, collisions;
    public final float speed, time, sectorTime, gold, silver, bronze, lastSectorTime, bestLap, skid;
    public final float steering, throttle, brake;
    private final int[] medals = new int[12];

    public RaceSnapshot(float[] v) {
        speed = v[0]; time = v[2]; state = (int) v[3]; countdown = (int) v[4]; completed = (int) v[5];
        sectorTime = v[6]; gold = v[7]; silver = v[8]; bronze = v[9]; lastSectorTime = v[10];
        lastMedal = (int) v[11]; rank = (int) v[12]; points = (int) v[13]; claimed = (int) v[14];
        bestLap = v[15]; skid = v[16]; collisions = (int) v[17];
        steering = v[19]; throttle = v[20]; brake = v[21];
        for (int i = 0; i < 12; i++) medals[i] = (int) v[22 + i];
    }
    public int medal(int index) { return medals[index]; }
    public int lap() { return Math.min(3, completed / 4 + 1); }
    public static String rankName(int medal) {
        return medal == 3 ? "GOLD" : medal == 2 ? "SILVER" : medal == 1 ? "BRONZE" : "UNRANKED";
    }
}
