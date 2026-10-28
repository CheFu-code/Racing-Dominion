"""Synthesize Racing Dominion's original effects; no external recordings or downloads.

Run with Python's standard library. PCM mono / 22050 Hz keeps the APK small.
"""
from pathlib import Path
import math
import random
import struct
import wave

RATE = 22050
OUTPUT = Path(__file__).resolve().parents[1] / "app/src/main/res/raw"


def save(name, duration, sample):
    OUTPUT.mkdir(parents=True, exist_ok=True)
    frames = bytearray()
    for i in range(round(duration * RATE)):
        value = max(-0.95, min(0.95, sample(i / RATE, i)))
        frames.extend(struct.pack("<h", round(value * 32767)))
    with wave.open(str(OUTPUT / f"rd_{name}.wav"), "wb") as output:
        output.setparams((1, 2, RATE, 0, "NONE", "not compressed"))
        output.writeframes(frames)


def tone(frequency, t):
    return math.sin(2 * math.pi * frequency * t)


def generate():
    random_source = random.Random(194)
    # Integer harmonics make a seamless one-second engine loop.
    save("engine", 1, lambda t, i: 0.23 * tone(80, t) + 0.14 * tone(160, t)
         + 0.08 * tone(240, t) + 0.04 * tone(480, t))
    noise = [random_source.uniform(-1, 1) for _ in range(RATE)]
    save("tire", 1, lambda t, i: (noise[i] - noise[(i - 1) % RATE]) * 0.13
         * min(1, t * 80, (1 - t) * 80) + 0.035 * tone(1700, t))
    save("impact", 0.24, lambda t, i: (noise[i] * 0.45 + tone(70, t) * 0.2) * math.exp(-t * 23))
    save("count", 0.16, lambda t, i: 0.35 * tone(660, t) * math.sin(math.pi * t / 0.16) ** 2)
    save("go", 0.30, lambda t, i: 0.3 * (tone(990, t) + 0.3 * tone(1485, t))
         * math.sin(math.pi * t / 0.30) ** 2)
    save("sector", 0.36, lambda t, i: 0.22 * tone(880 if t < 0.18 else 1320, t)
         * math.sin(math.pi * (t % 0.18) / 0.18) ** 2)
    notes = [523.25, 659.25, 783.99, 1046.50, 783.99, 1046.50]
    save("finish", 1.44, lambda t, i: 0.26 * tone(notes[min(5, int(t / 0.24))], t)
         * math.sin(math.pi * (t % 0.24) / 0.24) ** 2)
    print("Generated 7 original Racing Dominion sound effects.")


if __name__ == "__main__":
    generate()
