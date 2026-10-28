package co.za.chefu.racingdominion;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.view.View;
import java.util.Locale;

/** Lightweight race overlay; real Android buttons provide pause and sound actions. */
public final class RacingHudView extends View {
    private static final int WHITE = 0xFFF6EEFF, MUTED = 0xFFBEABC9;
    private static final int ORANGE = 0xFFFF6B35, ICE = 0xFF8DE8FF, PANEL = 0xED24112F;
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final RectF rect = new RectF();
    private final Path circuit = new Path();
    private final Typeface bold = Typeface.create("sans-serif-condensed", Typeface.BOLD);
    private final Typeface regular = Typeface.create("sans-serif-medium", Typeface.NORMAL);
    private RaceSnapshot race = new RaceSnapshot(new float[34]);
    private float width = 800, height = 400, bestTime;
    private int bestRank;
    private boolean personalBest;
    private LinearGradient menuShade, raceShade;

    public RacingHudView(Context context) {
        super(context);
        setClickable(false);
        setFocusable(false);
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
    public void update(RaceSnapshot race, float bestTime, int bestRank, boolean personalBest) {
        this.race = race; this.bestTime = bestTime; this.bestRank = bestRank; this.personalBest = personalBest;
        setContentDescription(race.state == RaceSnapshot.MENU
                ? "Racing Dominion. Claim the Circuit. Three laps, twelve sectors. Tap the track to start. "
                  + "Drag left or right on the left steering pad. Hold the upper right pedal to accelerate, lower pedal to brake. Use both thumbs."
                : (race.state == RaceSnapshot.COUNTDOWN ? "Starting in " + race.countdown + ". " : "")
                  + "Lap " + race.lap() + " of 3. " + race.claimed + " sectors claimed. " + race.points + " points. "
                  + (race.state == RaceSnapshot.FINISHED ? RaceSnapshot.rankName(race.rank) + " Dominion rank. Tap to retry." : ""));
        invalidate();
    }
    @Override protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        if (w == 0 || h == 0) return;
        float scale = Math.min(w / 800f, h / 400f);
        width = w / scale; height = h / scale;
        menuShade = new LinearGradient(0, 0, w, 0,
                new int[]{0xFA180A22, 0xE624112F, 0x4024112F}, new float[]{0, 0.5f, 1}, Shader.TileMode.CLAMP);
        raceShade = new LinearGradient(0, 0, 0, h,
                new int[]{0xB0180A22, Color.TRANSPARENT, 0xA024112F}, new float[]{0, 0.4f, 1}, Shader.TileMode.CLAMP);
    }
    @Override protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        paint.setShader(race.state == RaceSnapshot.MENU ? menuShade : raceShade);
        canvas.drawRect(0, 0, getWidth(), getHeight(), paint);
        paint.setShader(null);
        canvas.save(); canvas.scale(getWidth() / width, getHeight() / height);
        if (race.state == RaceSnapshot.MENU) drawMenu(canvas);
        else {
            drawRace(canvas);
            if (race.state == RaceSnapshot.FINISHED) drawResult(canvas);
            if (race.state == RaceSnapshot.PAUSED) drawPause(canvas);
            if (race.state == RaceSnapshot.COUNTDOWN) drawCountdown(canvas);
        }
        canvas.restore();
    }
    private void drawMenu(Canvas c) {
        text(c, "RD / RACING DOMINION", 28, 34, 14, ORANGE, true);
        float y = height / 2 - 85;
        text(c, "CLAIM THE", 30, y + 40, 46, WHITE, true);
        text(c, "CIRCUIT.", 30, y + 88, 52, ORANGE, true);
        text(c, "12 sectors. Three laps. Earn your Dominion rank.", 32, y + 115, 12, MUTED, false);
        text(c, "GOLD 30+  /  SILVER 20+  /  BRONZE 10+ PTS", 32, y + 141, 11, ICE, true);
        text(c, bestTime > 0 ? "PB  " + clock(bestTime) + "  /  BEST RANK " + RaceSnapshot.rankName(bestRank)
                : "RD-01 / FOUNDRY LIVERY / CROWNLINE", 32, y + 165, 11, MUTED, false);
        drawCircuitCard(c);
        panel(c, 32, height - 73, 245, 43, ORANGE);
        text(c, "TAP TRACK TO START  >", 50, height - 46, 15, 0xFF180A22, true);
        text(c, "LEFT THUMB / DRAG TO STEER", width - 275, height - 59, 11, WHITE, true);
        text(c, "RIGHT THUMB / GAS + BRAKE", width - 275, height - 39, 11, ICE, false);
    }
    private void drawRace(Canvas c) {
        panel(c, 24, 20, 112, 63, PANEL);
        text(c, "LAP / CROWNLINE", 36, 40, 9, MUTED, true);
        text(c, "0" + race.lap(), 36, 69, 28, WHITE, true);
        text(c, "/ 03", 76, 67, 13, ICE, false);
        panel(c, 150, 20, 270, 63, PANEL);
        int sector = Math.min(12, race.completed + 1);
        text(c, "SECTOR " + sector + "/12   /   " + String.format(Locale.US, "%.2fs", race.sectorTime), 164, 40, 12, WHITE, true);
        text(c, String.format(Locale.US, "G %.1f   S %.1f   B %.1fs", race.gold, race.silver, race.bronze), 164, 61, 12, ICE, false);
        text(c, race.claimed + "/12 CLAIMED   /   " + race.points + "/36 POINTS", 164, 76, 9, MUTED, true);
        panel(c, width - 190, 20, 166, 63, PANEL);
        text(c, "RACE TIME", width - 176, 40, 9, MUTED, true);
        text(c, clock(race.time), width - 176, 69, 27, WHITE, true);
        float x = width / 2 - 90, y = height - 132;
        panel(c, x, y, 180, 78, PANEL);
        text(c, Integer.toString(Math.round(race.speed * 3.6f)), x + 16, y + 47, 41, WHITE, true);
        text(c, "KM/H", x + 126, y + 27, 10, ICE, true);
        text(c, "RD-01", x + 126, y + 43, 9, MUTED, false);
        panel(c, x + 16, y + 60, 148, 4, 0xFF513653);
        panel(c, x + 16, y + 60, Math.min(148, race.speed / 80 * 148), 4, ORANGE);
        drawMedals(c, width / 2 - 138, height - 35, 19, 4);
        if (race.completed > 0 && race.sectorTime < 2.3f && race.state == RaceSnapshot.RACING) {
            text(c, "SECTOR " + race.completed + " / " + RaceSnapshot.rankName(race.lastMedal)
                    + String.format(Locale.US, " / %.2fs", race.lastSectorTime), width / 2 - 139, 111, 13,
                    medalColor(race.lastMedal), true);
        }
        if (race.state == RaceSnapshot.RACING && race.time < 0.7f)
            centered(c, "GO", width / 2, height / 2, 44, ICE);
        // These positions match TouchControls' steering origin and pedal boundary.
        paint.setColor(0xAA24112F); c.drawCircle(90, height - 90, 59, paint);
        text(c, "<", 40, height - 84, 22, ICE, true);
        text(c, ">", 128, height - 84, 22, ICE, true);
        paint.setColor(race.steering == 0 ? MUTED : ORANGE);
        c.drawCircle(90 - race.steering * 38, height - 90, 17, paint);
        text(c, "DRAG TO STEER", 46, height - 17, 10, MUTED, true);
        pedal(c, height - 158, "+ ACCELERATE", race.throttle > 0, ORANGE);
        pedal(c, height - 94, "- BRAKE", race.brake > 0, ICE);
    }
    private void drawCountdown(Canvas c) {
        panel(c, width / 2 - 87, height / 2 - 65, 174, 115, PANEL);
        text(c, "READY TO CLAIM", width / 2 - 59, height / 2 - 40, 12, ICE, true);
        centered(c, Integer.toString(race.countdown), width / 2, height / 2 + 25, 64, ORANGE);
    }
    private void drawPause(Canvas c) {
        shade(c);
        panel(c, width / 2 - 190, height / 2 - 68, 380, 136, PANEL);
        centered(c, "RACE PAUSED", width / 2, height / 2 - 14, 32, WHITE);
        centered(c, "Tap the track or RESUME to continue", width / 2, height / 2 + 25, 14, ICE);
    }
    private void drawResult(Canvas c) {
        shade(c);
        float x = width / 2 - 245, y = height / 2 - 130;
        panel(c, x, y, 490, 260, PANEL);
        text(c, "CLAIM THE CIRCUIT / RESULTS", x + 24, y + 26, 11, ICE, true);
        text(c, race.rank > 0 ? RaceSnapshot.rankName(race.rank) + " DOMINION" : "RUN COMPLETE", x + 24, y + 64, 34, medalColor(race.rank), true);
        text(c, "TOTAL " + clock(race.time) + "   /   BEST LAP " + clock(race.bestLap), x + 24, y + 93, 14, WHITE, true);
        text(c, race.claimed + "/12 SECTORS CLAIMED   /   " + race.points + "/36 POINTS", x + 24, y + 119, 13, ICE, true);
        drawMedals(c, x + 24, y + 136, 30, 5);
        text(c, personalBest ? "NEW PERSONAL BEST" : "GOLD 30+ / SILVER 20+ / BRONZE 10+ POINTS", x + 24, y + 185, 11, ORANGE, true);
        panel(c, x + 24, y + 203, 442, 37, ORANGE);
        centered(c, "TAP TRACK TO CLAIM AGAIN  >", width / 2, y + 227, 14, 0xFF180A22);
    }
    private void drawMedals(Canvas c, float x, float y, float size, float gap) {
        for (int i = 0; i < 12; i++) {
            panel(c, x + i * (size + gap), y, size, 16,
                    i < race.completed ? medalColor(race.medal(i)) : 0xFF513653);
        }
    }
    private void drawCircuitCard(Canvas c) {
        float x = width - 278, y = height / 2 - 112;
        panel(c, x, y, 250, 180, PANEL);
        text(c, "CROWNLINE / FOUNDRY DISTRICT", x + 17, y + 26, 11, WHITE, true);
        circuit.rewind();
        for (int i = 0; i <= 96; i++) {
            double a = i * Math.PI * 2 / 96;
            float px = x + 125 + (float) (72 * Math.cos(a) + 12 * Math.cos(2 * a) - 8);
            float py = y + 92 - (float) (52 * Math.sin(a) + 9 * Math.sin(3 * a)) * 0.8f;
            if (i == 0) circuit.moveTo(px, py); else circuit.lineTo(px, py);
        }
        paint.setStyle(Paint.Style.STROKE); paint.setStrokeWidth(8); paint.setColor(0xFF513653); c.drawPath(circuit, paint);
        paint.setStrokeWidth(2); paint.setColor(ORANGE); c.drawPath(circuit, paint); paint.setStyle(Paint.Style.FILL);
        for (int i = 0; i < 4; i++) {
            double a = i * Math.PI / 2;
            paint.setColor(ICE);
            c.drawCircle(x + 125 + (float) (72 * Math.cos(a) + 12 * Math.cos(2 * a) - 8),
                    y + 92 - (float) (52 * Math.sin(a) + 9 * Math.sin(3 * a)) * 0.8f, 4, paint);
        }
        text(c, "4 SECTORS / LAP", x + 18, y + 161, 10, ICE, true);
        text(c, "3 LAPS", x + 190, y + 161, 10, MUTED, false);
    }
    private void pedal(Canvas c, float y, String label, boolean active, int color) {
        panel(c, width - 160, y, 136, 46, active ? color : PANEL);
        text(c, label, width - 146, y + 28, 12, active ? 0xFF180A22 : color, true);
    }
    private void shade(Canvas c) { paint.setColor(0xC0180A22); c.drawRect(0, 0, width, height, paint); }
    private static int medalColor(int rank) { return rank == 3 ? 0xFFFFD166 : rank == 2 ? 0xFFD9E7F5 : rank == 1 ? 0xFFD99562 : 0xFF8D708E; }
    public static String clock(float value) { return String.format(Locale.US, "%d:%05.2f", (int) (value / 60), value % 60); }
    private void panel(Canvas c, float x, float y, float w, float h, int color) {
        paint.setColor(color); rect.set(x, y, x + w, y + h); c.drawRoundRect(rect, 8, 8, paint);
    }
    private void text(Canvas c, String text, float x, float y, float size, int color, boolean heavy) {
        paint.setColor(color); paint.setTypeface(heavy ? bold : regular); paint.setTextSize(size); c.drawText(text, x, y, paint);
    }
    private void centered(Canvas c, String text, float x, float y, float size, int color) {
        paint.setTypeface(bold); paint.setTextSize(size);
        text(c, text, x - paint.measureText(text) / 2, y, size, color, true);
    }
}
