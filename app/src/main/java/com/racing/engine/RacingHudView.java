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

/** Decorative overlay: all touch events continue to the game surface beneath it. */
public final class RacingHudView extends View {
    private static final int WHITE = 0xFFF3F6FA;
    private static final int MUTED = 0xFFABB7C7;
    private static final int LIME = 0xFFD4FF55;
    private static final int PANEL = 0xE6101824;
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final RectF rect = new RectF();
    private final Path circuit = new Path();
    private final Typeface bold = Typeface.create("sans-serif-condensed", Typeface.BOLD);
    private final Typeface regular = Typeface.create("sans-serif-medium", Typeface.NORMAL);
    private int state;
    private int speed;
    private int lap = 1;
    private String clock = "0:00.00";
    private float width;
    private float height;
    private LinearGradient menuShade;
    private LinearGradient raceShade;

    public RacingHudView(Context context) {
        super(context);
        setClickable(false);
        setFocusable(false);
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    public void update(RacingRenderer renderer) {
        state = renderer.getGameState();
        speed = Math.max(0, Math.round(renderer.getCurrentSpeed() * 3.6f));
        lap = Math.max(1, Math.min(3, renderer.getCurrentLap() + 1));
        float time = Math.max(0, renderer.getRaceTime());
        clock = String.format(Locale.US, "%d:%05.2f", (int) (time / 60), time % 60);
        if (state == 0) {
            setContentDescription("Racing Dominion. Three lap circuit race. Tap anywhere to start. "
                    + "Left half: top steers left, bottom steers right. "
                    + "Right half: top accelerates, bottom brakes.");
        } else {
            setContentDescription((state == 4 ? "Race complete. Tap anywhere to race again. " : "")
                    + "Lap " + lap + " of 3. Speed " + speed + " kilometres per hour. Time " + clock);
        }
        invalidate();
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        // Fit a landscape design to both small phones and wide tablets.
        float scale = Math.min(w / 800f, h / 400f);
        width = w / scale;
        height = h / scale;
        menuShade = new LinearGradient(0, 0, w, 0,
                new int[]{0xFA080E18, 0xD9080E18, 0x30080E18},
                new float[]{0, 0.48f, 1}, Shader.TileMode.CLAMP);
        raceShade = new LinearGradient(0, 0, 0, h,
                new int[]{0xB0080E18, Color.TRANSPARENT, 0x90080E18},
                new float[]{0, 0.4f, 1}, Shader.TileMode.CLAMP);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        paint.setShader(state == 0 ? menuShade : raceShade);
        canvas.drawRect(0, 0, getWidth(), getHeight(), paint);
        paint.setShader(null);
        canvas.save();
        canvas.scale(getWidth() / width, getHeight() / height);
        if (state == 0) {
            drawMenu(canvas);
        } else {
            drawRace(canvas);
            if (state == 3 || state == 4) drawResult(canvas);
        }
        canvas.restore();
    }

    private void drawMenu(Canvas canvas) {
        text(canvas, "RD / RACING DOMINION", 28, 33, 13, LIME, true);
        text(canvas, "CROWNLINE / 01", width - 122, 33, 11, WHITE, false);
        float y = height / 2 - 67;
        text(canvas, "CLAIM THE", 30, y + 40, 48, WHITE, true);
        text(canvas, "CROWNLINE.", 30, y + 88, 48, LIME, true);
        text(canvas, "Three laps. One track. Make every second count.",
                32, y + 115, 13, MUTED, false);
        panel(canvas, 32, y + 136, 188, 32, 0xC81A2532);
        text(canvas, "RD-01   /   CIRCUIT RACE", 45, y + 157, 11, WHITE, true);
        drawCircuitCard(canvas);
        panel(canvas, 32, height - 78, 240, 45, LIME);
        text(canvas, "TAP ANYWHERE TO RACE", 49, height - 50, 14, 0xFF101820, true);
        text(canvas, ">", 247, height - 50, 16, 0xFF101820, true);
        text(canvas, "LEFT HALF  /  STEER", width - 242, height - 66, 11, WHITE, true);
        text(canvas, "RIGHT HALF  /  GAS + BRAKE", width - 242, height - 45, 11, MUTED, false);
    }

    private void drawRace(Canvas canvas) {
        panel(canvas, 24, 20, 126, 62, PANEL);
        text(canvas, "LAP", 38, 42, 10, MUTED, true);
        text(canvas, String.format(Locale.US, "%02d", lap), 38, 69, 27, WHITE, true);
        text(canvas, "/ 03", 77, 68, 14, MUTED, false);
        for (int i = 0; i < 3; i++) {
            panel(canvas, 114 + i * 8, 51, 4, 17, i < lap ? LIME : 0xFF364150);
        }
        text(canvas, "RACING DOMINION", width / 2 - 58, 35, 12, WHITE, true);
        text(canvas, "CROWNLINE / 01", width / 2 - 35, 52, 9, MUTED, false);
        panel(canvas, width - 174, 20, 150, 62, PANEL);
        text(canvas, "RACE TIME", width - 160, 42, 10, MUTED, true);
        text(canvas, clock, width - 160, 69, 26, WHITE, true);

        float x = width / 2 - 83;
        float y = height - 96;
        panel(canvas, x, y, 166, 76, PANEL);
        text(canvas, Integer.toString(speed), x + 17, y + 48, 42, WHITE, true);
        text(canvas, "KM/H", x + 111, y + 28, 10, LIME, true);
        text(canvas, "SPEED", x + 111, y + 43, 9, MUTED, false);
        for (int i = 0; i < 20; i++) {
            panel(canvas, x + 17 + i * 6.7f, y + 59, 4.5f, 4,
                    i < speed / 288f * 20 ? LIME : 0xFF364150);
        }
        // Positions match the engine's existing vertical control zones.
        guide(canvas, 24, height / 2 - 47, "<", "STEER LEFT", false);
        guide(canvas, 24, height / 2 + 20, ">", "STEER RIGHT", false);
        guide(canvas, width - 153, height / 2 - 47, "+", "ACCELERATE", true);
        guide(canvas, width - 153, height / 2 + 20, "-", "BRAKE", true);
    }

    private void drawResult(Canvas canvas) {
        paint.setColor(0xB0080E18);
        canvas.drawRect(0, 0, width, height, paint);
        float x = width / 2 - 170;
        float y = height / 2 - 77;
        panel(canvas, x, y, 340, 154, 0xFA101824);
        text(canvas, state == 4 ? "CHEQUERED FLAG" : "RACE ON HOLD", x + 24, y + 30, 11, LIME, true);
        text(canvas, state == 4 ? "RACE COMPLETE" : "PAUSED", x + 24, y + 66, 32, WHITE, true);
        text(canvas, "TOTAL TIME  /  " + clock, x + 24, y + 94, 16, MUTED, false);
        if (state == 4) {
            panel(canvas, x + 24, y + 109, 292, 29, LIME);
            text(canvas, "TAP ANYWHERE TO RACE AGAIN", x + 41, y + 129, 12, 0xFF101820, true);
        }
    }

    private void drawCircuitCard(Canvas canvas) {
        float x = width - 278;
        float y = height / 2 - 112;
        panel(canvas, x, y, 250, 180, 0xC8101824);
        text(canvas, "01 / CROWNLINE CIRCUIT", x + 18, y + 26, 12, WHITE, true);
        circuit.rewind();
        for (int i = 0; i <= 96; ++i) {
            double a = i * Math.PI * 2 / 96;
            float px = x + 125 + (float) (72 * Math.cos(a) + 12 * Math.cos(2 * a) - 8);
            float py = y + 92 - (float) (52 * Math.sin(a) + 9 * Math.sin(3 * a)) * 0.8f;
            if (i == 0) circuit.moveTo(px, py); else circuit.lineTo(px, py);
        }
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(9);
        paint.setColor(0xFF31414D);
        canvas.drawPath(circuit, paint);
        paint.setStrokeWidth(2);
        paint.setColor(LIME);
        canvas.drawPath(circuit, paint);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(WHITE);
        canvas.drawCircle(x + 201, y + 92, 4, paint);
        text(canvas, "RD-01 PROTOTYPE", x + 18, y + 161, 10, LIME, true);
        text(canvas, "3 LAPS", x + 190, y + 161, 10, MUTED, false);
    }

    private void guide(Canvas canvas, float x, float y, String symbol, String label, boolean pedal) {
        panel(canvas, x, y, 129, 32, 0x80101824);
        text(canvas, symbol, x + 12, y + 23, 22, pedal ? LIME : WHITE, true);
        text(canvas, label, x + 37, y + 20, 10, WHITE, true);
    }

    private void panel(Canvas canvas, float x, float y, float w, float h, int color) {
        paint.setColor(color);
        rect.set(x, y, x + w, y + h);
        canvas.drawRoundRect(rect, 8, 8, paint);
    }

    private void text(Canvas canvas, String text, float x, float y, float size, int color, boolean heavy) {
        paint.setColor(color);
        paint.setTypeface(heavy ? bold : regular);
        paint.setTextSize(size);
        canvas.drawText(text, x, y, paint);
    }
}
