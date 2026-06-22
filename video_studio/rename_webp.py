import os
from pathlib import Path

effect_dir = Path("c:/we_hunting/video_studio/assets/effect")
webp_files = list(effect_dir.glob("*.webp"))
count = 0

for webp_file in webp_files:
    png_file = webp_file.with_suffix(".png")
    if png_file.exists():
        webp_file.unlink() # remove if png already exists
    else:
        webp_file.rename(png_file)
    count += 1

print(f"Renamed {count} files to .png")
