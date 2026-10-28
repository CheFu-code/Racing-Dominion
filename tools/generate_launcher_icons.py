"""Render the authored RD racing monogram for legacy Android launchers.

Keep these polygons in sync with drawable/ic_launcher_foreground.xml.
Requires Pillow. Run from any directory; output paths are relative to this repo.
"""
from pathlib import Path

from PIL import Image, ImageDraw

RES = Path(__file__).resolve().parents[1] / "app/src/main/res"
DENSITIES = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}
ORANGE = "#FF6B35"
ICE_BLUE = "#8DE8FF"
PLUM = "#24112F"


def create_icon(size: int, rounded: bool = False) -> Image.Image:
    # Supersampling keeps diagonal edges crisp at the smaller densities.
    render_size = size * 4
    image = Image.new("RGBA", (render_size, render_size), PLUM)
    draw = ImageDraw.Draw(image)
    scale = render_size / 108
    shapes = [
        (ORANGE, [(29, 35), (47, 35), (55, 43), (55, 52), (49, 58), (57, 72),
                (46, 72), (39, 60), (38, 60), (38, 72), (29, 72)]),
        (PLUM, [(38, 43), (38, 52), (44, 52), (47, 49), (47, 46), (44, 43)]),
        (ICE_BLUE, [(59, 35), (69, 35), (80, 46), (80, 61), (69, 72), (59, 72)]),
        (PLUM, [(67, 44), (67, 63), (72, 58), (72, 49)]),
        (ORANGE, [(37, 76), (45, 76), (42, 79), (34, 79)]),
        (ORANGE, [(49, 76), (72, 76), (69, 79), (46, 79)]),
    ]
    for color, polygon in shapes:
        draw.polygon([(x * scale, y * scale) for x, y in polygon], fill=color)
    if rounded:
        mask = Image.new("L", image.size)
        ImageDraw.Draw(mask).ellipse((0, 0, render_size - 1, render_size - 1), fill=255)
        image.putalpha(mask)
    return image.resize((size, size), Image.Resampling.LANCZOS)


if __name__ == "__main__":
    for density, size in DENSITIES.items():
        directory = RES / f"mipmap-{density}"
        directory.mkdir(parents=True, exist_ok=True)
        create_icon(size).save(directory / "ic_launcher.png")
        create_icon(size, rounded=True).save(directory / "ic_launcher_round.png")
        print(f"Generated Racing Dominion {density} icons ({size}x{size})")
