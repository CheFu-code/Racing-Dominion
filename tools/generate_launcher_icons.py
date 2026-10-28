"""Render the authored Dominion crown/road emblem for legacy Android launchers.

Keep these polygons in sync with drawable/ic_launcher_foreground.xml.
Requires Pillow. Run from any directory; output paths are relative to this repo.
"""
from pathlib import Path

from PIL import Image, ImageDraw

RES = Path(__file__).resolve().parents[1] / "app/src/main/res"
DENSITIES = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}
LIME = "#D4FF55"
WHITE = "#F3F6FA"


def create_icon(size: int, rounded: bool = False) -> Image.Image:
    # Supersampling keeps diagonal edges crisp at the smaller densities.
    render_size = size * 4
    image = Image.new("RGBA", (render_size, render_size), "#101824")
    draw = ImageDraw.Draw(image)
    scale = render_size / 108
    shapes = [
        (LIME, [(32, 39), (43, 48), (54, 32), (65, 48), (76, 39), (70, 61), (38, 61)]),
        (WHITE, [(43, 66), (50, 66), (46, 77), (36, 77)]),
        (WHITE, [(58, 66), (65, 66), (72, 77), (62, 77)]),
        (LIME, [(53, 66), (55, 66), (56, 71), (52, 71)]),
        (LIME, [(52, 74), (56, 74), (57, 78), (51, 78)]),
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
